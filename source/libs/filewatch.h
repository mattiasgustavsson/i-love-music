


typedef struct filewatch_t filewatch_t;

filewatch_t* filewatch_create( char const* path );

bool filewatch_check( filewatch_t* watch, int timeout_ms );
bool filewatch_multi_check( filewatch_t** watches, int count, int timeout_ms );

void filewatch_destroy( filewatch_t* watch );


#ifdef FILEWATCH_IMPLEMENTATION

#include <windows.h>

struct filewatch_t
    {
    HANDLE handle;
    char path[ 260 ];
    };

filewatch_t* filewatch_create( char const* path )
    {
    filewatch_t* watch = (filewatch_t*) malloc( sizeof( filewatch_t ) );
    strcpy( watch->path, path );
    watch->handle = FindFirstChangeNotificationA( path, TRUE, 
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION );
    return watch;
    }

bool filewatch_check( filewatch_t* watch, int timeout_ms )
    {
    
    DWORD wait_status = WaitForSingleObject( watch->handle, (DWORD) timeout_ms ); 
    bool result = ( wait_status == WAIT_OBJECT_0 );
    FindNextChangeNotification( watch->handle );
    return result;
    }


bool filewatch_multi_check( filewatch_t** watches, int count, int timeout_ms )
    {
    if( count > MAXIMUM_WAIT_OBJECTS ) count = MAXIMUM_WAIT_OBJECTS;
    HANDLE handles[ MAXIMUM_WAIT_OBJECTS ];
    for( int i = 0; i < count; ++i )
        handles[ i ] = watches[ i ]->handle; 
    DWORD wait_status = WaitForMultipleObjects( count, handles, FALSE, (DWORD) timeout_ms ); 
    bool result = ( wait_status != WAIT_TIMEOUT && wait_status != WAIT_FAILED );
    if( result ) 
        {
        for( int i = 0; i < count; ++i )
            {
            FindCloseChangeNotification( watches[ i ]->handle );
            watches[ i ]->handle = FindFirstChangeNotificationA( watches[ i ]->path, TRUE, 
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION );
            }
        }
    return result;
    }


void filewatch_destroy( filewatch_t* watch )
    {
    FindCloseChangeNotification( watch->handle );
    free( watch );
    }


#endif /* MMAP_IMPLEMENTATION */