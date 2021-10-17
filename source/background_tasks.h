
typedef struct background_task_t
    {
    int priority;
    void (*execute)( void* );
    void (*cancel)( void* );
    void* user_data;
    } background_task_t;


typedef struct background_tasks_t
    {
    thread_ptr_t thread;
    thread_atomic_int_t exit_flag;
    thread_signal_t new_tasks_signal;
    
    thread_mutex_t tasks_mutex;    
    int tasks_count;
    int tasks_capacity;
    background_task_t* tasks;
    } background_tasks_t;


static int background_thread_proc( void* user_data )
    {
    background_tasks_t* tasks = (background_tasks_t*) user_data;

    thread_timer_t timer;
    thread_timer_init( &timer );

    while( thread_atomic_int_load( &tasks->exit_flag ) == 0 )
        {
        thread_mutex_lock( &tasks->tasks_mutex );
        while( tasks->tasks_count > 0 && thread_atomic_int_load( &tasks->exit_flag ) == 0 )
            {
            background_task_t task = tasks->tasks[ 0 ];
            memmove( tasks->tasks, tasks->tasks + 1, ( --tasks->tasks_count ) * sizeof( *tasks->tasks ) );
            thread_mutex_unlock( &tasks->tasks_mutex );
            task.execute( task.user_data );
    
            thread_timer_wait( &timer, 5000000 ); // sleep for 5 milliseconds (throttle background thread a bit)
            thread_mutex_lock( &tasks->tasks_mutex );
            }
        thread_mutex_unlock( &tasks->tasks_mutex );

        thread_signal_wait( &tasks->new_tasks_signal, 10000 );
        }

    thread_timer_term( &timer );
    return 0;
    }


void background_tasks_init( background_tasks_t* tasks )
    {
    memset( tasks, 0, sizeof( *tasks ) );

    thread_mutex_init( &tasks->tasks_mutex );
    tasks->tasks_count = 0;
    tasks->tasks_capacity = 1024;
    tasks->tasks = (background_task_t*) malloc( sizeof( background_task_t ) * tasks->tasks_capacity );

    thread_atomic_int_store( &tasks->exit_flag, 0 );
    thread_signal_init( &tasks->new_tasks_signal );
    tasks->thread = thread_create( background_thread_proc, tasks, 0, THREAD_STACK_SIZE_DEFAULT );
    }


void background_tasks_term( background_tasks_t* tasks )
    {
    thread_atomic_int_store( &tasks->exit_flag, 1 );
    thread_signal_raise( &tasks->new_tasks_signal );
    thread_join( tasks->thread );
    thread_destroy( tasks->thread );
    thread_signal_term( &tasks->new_tasks_signal );
    thread_mutex_term( &tasks->tasks_mutex );
    for( int i = 0; i < tasks->tasks_count; ++i ) 
        if( tasks->tasks[ i ].cancel ) tasks->tasks[ i ].cancel( tasks->tasks[ i ].user_data );
    free( tasks->tasks );
    }


void background_tasks_add( background_tasks_t* tasks, int priority, void (*execute)( void* ), void (*cancel)( void* ), void* user_data )
    {
    thread_mutex_lock( &tasks->tasks_mutex );
    
    if( tasks->tasks_count >= tasks->tasks_capacity )
        {
        tasks->tasks_capacity *= 2;
        background_task_t* new_tasks = (background_task_t*) malloc( sizeof( background_task_t ) * tasks->tasks_capacity );
        memcpy( new_tasks, tasks->tasks, sizeof( background_task_t ) * tasks->tasks_count );
        free( tasks->tasks );
        tasks->tasks = new_tasks;
        }
   
    int index = -1;
    for( int i = 0; i < tasks->tasks_count; ++i )
        {
        if( tasks->tasks[ i ].priority < priority )
            {
            index = i;
            break;
            }
        }

    background_task_t task;
    task.priority = priority;
    task.execute = execute;
    task.cancel = cancel;
    task.user_data = user_data;
    if( index < 0 )
        {
        tasks->tasks[ tasks->tasks_count++ ] = task;
        }
    else
        {
        memmove( tasks->tasks + index + 1, tasks->tasks + index, sizeof( background_task_t ) * ( tasks->tasks_count - index ) );
        tasks->tasks[ index ] = task;
        ++tasks->tasks_count;
        }

    thread_mutex_unlock( &tasks->tasks_mutex );

    thread_signal_raise( &tasks->new_tasks_signal );
    }


int background_tasks_count( background_tasks_t* tasks )
    {
    thread_mutex_lock( &tasks->tasks_mutex );
    int value = tasks->tasks_count;
    thread_mutex_unlock( &tasks->tasks_mutex );
    return value;
    }