
void swapptr( APP_S16** a, APP_S16** b )
    {
    APP_S16* t = *a;
    *a = *b;
    *b = t;
    }


void swapint( int* a, int* b )
    {
    int t = *a;
    *a = *b;
    *b = t;
    }


void swapstr( char* a, char* b )
    {
    char t[ 256 ];
    strcpy( t, a );
    strcpy( a, b );
    strcpy( b, "" );
    }


void swapflt( float* a, float* b )
    {
    float t = *a;
    *a = *b;
    *b = t;
    }


typedef struct play_thread_t
    {
    thread_mutex_t mutex;
    thread_signal_t signal;
    thread_atomic_int_t exit_flag;
    thread_ptr_t thread_handle;

    bool loop;
    bool resume_after_decode;
    bool ignore_resume;
    
    bool is_paused;
    bool was_paused;
    bool is_ffwd;
    bool is_rewind;
    int ffwd_speed;
    int rewind_speed;

    char next_song[ 256 ];
    float next_song_gain;
    char current_song[ 256 ];
    float current_song_gain;

    char buffered_next_song[ 256 ];
    float buffered_next_song_gain;
    char buffered_current_song[ 256 ];
    float buffered_current_song_gain;

    bool set_position_activated;
    float set_position;
    bool song_finished;
    bool is_decoding;

    thread_mutex_t samples_mutex;
    APP_S16* sample_pairs;
    int sample_pairs_count;
    int sample_pairs_capacity;
    int position;
    float track_gain;

    APP_S16* next_song_sample_pairs;
    int next_song_sample_pairs_count;
    int next_song_sample_pairs_capacity;
    float next_song_track_gain;
    } play_thread_t;


void sound_proc( APP_S16* sample_pairs, int sample_pairs_count, void* user_data )
    {
    play_thread_t* play = (play_thread_t*) user_data;
    thread_mutex_lock( &play->samples_mutex );
    
    bool fade_in = play->was_paused && !play->is_paused;
    bool fade_out = !play->was_paused && play->is_paused;
    play->was_paused = play->is_paused;
    if( play->sample_pairs_count > 0 )
        {
        if( play->is_ffwd )
            {
            while( sample_pairs_count )
                {
                int count = play->sample_pairs_count - play->position;
                if( count > sample_pairs_count * play->ffwd_speed ) count = sample_pairs_count * play->ffwd_speed;
                int j = 0;
                for( int i = 0; i < count; i += play->ffwd_speed, ++j )
                    {
                    sample_pairs[ j * 2 + 0 ] = (APP_S16)( ( play->sample_pairs[ play->position * 2 + i * 2 + 0 ] / 4 ) * play->track_gain );
                    sample_pairs[ j * 2 + 1 ] = (APP_S16)( ( play->sample_pairs[ play->position * 2 + i * 2 + 1 ] / 4 ) * play->track_gain );
                    }
                sample_pairs += count * 2;
                sample_pairs_count -= ( count + ( play->ffwd_speed - 1 ) ) / play->ffwd_speed;
                play->position += count;        
                if( play->position >= play->sample_pairs_count )
                    {
                    play->position = play->sample_pairs_count - 1;          
                    break;
                    }
                }   
            }
        else if( play->is_rewind )
            {
            while( sample_pairs_count )
                {
                int count = play->position;
                if( count > sample_pairs_count * play->rewind_speed ) count = sample_pairs_count * play->rewind_speed;
                int j = 0;
                for( int i = 0; i < count; i += play->rewind_speed, ++j )
                    {
                    sample_pairs[ j * 2 + 0 ] = (APP_S16)( ( play->sample_pairs[ play->position * 2 - i * 2 + 0 ] / 4 ) * play->track_gain );
                    sample_pairs[ j * 2 + 1 ] = (APP_S16)( ( play->sample_pairs[ play->position * 2 - i * 2 + 1 ] / 4 ) * play->track_gain );
                    }
                sample_pairs += count * 2;
                sample_pairs_count -= ( count + ( play->rewind_speed - 1 ) ) / play->rewind_speed;
                play->position -= count;        
                if( play->position <= 0 )
                    {
                    play->position = 0;          
                    break;
                    }
                }   
            }
        else if( fade_out )
            {
            int mute_max = sample_pairs_count;
            int mute_level = mute_max;
            while( sample_pairs_count )
                {
                int count = play->sample_pairs_count - play->position;
                if( count > sample_pairs_count ) count = sample_pairs_count;
                for( int i = 0; i < count; ++i )
                    {
                    sample_pairs[ i * 2 + 0 ] = (APP_S16)( ( ( play->sample_pairs[ play->position * 2 + i * 2 + 0 ] * mute_level ) / mute_max ) * play->track_gain ) ;
                    sample_pairs[ i * 2 + 1 ] = (APP_S16)( ( ( play->sample_pairs[ play->position * 2 + i * 2 + 1 ] * mute_level ) / mute_max ) * play->track_gain ) ;
                    --mute_level;
                    }
                sample_pairs += count * 2;
                sample_pairs_count -= count;
                play->position += count;        
                if( play->position >= play->sample_pairs_count )
                    {
                    play->position = 0;          
                    swapstr( play->buffered_current_song, play->buffered_next_song );
                    swapflt( &play->buffered_current_song_gain, &play->buffered_next_song_gain );
                    swapptr( &play->sample_pairs, &play->next_song_sample_pairs );
                    swapint( &play->sample_pairs_capacity, &play->next_song_sample_pairs_capacity );
                    swapint( &play->sample_pairs_count, &play->next_song_sample_pairs_count );
                    swapflt( &play->track_gain, &play->next_song_track_gain );
                    play->song_finished = true;
                    }
                }   
            }
        else if( fade_in )
            {
            int mute_max = sample_pairs_count;
            int mute_level = 0;
            while( sample_pairs_count )
                {
                int count = play->sample_pairs_count - play->position;
                if( count > sample_pairs_count ) count = sample_pairs_count;
                for( int i = 0; i < count; ++i )
                    {
                    sample_pairs[ i * 2 + 0 ] = (APP_S16)( ( ( play->sample_pairs[ play->position * 2 + i * 2 + 0 ] * mute_level ) / mute_max ) * play->track_gain ) ;
                    sample_pairs[ i * 2 + 1 ] = (APP_S16)( ( ( play->sample_pairs[ play->position * 2 + i * 2 + 1 ] * mute_level ) / mute_max ) * play->track_gain ) ;
                    ++mute_level;
                    }
                sample_pairs += count * 2;
                sample_pairs_count -= count;
                play->position += count;        
                if( play->position >= play->sample_pairs_count )
                    {
                    play->position = 0;          
                    swapstr( play->buffered_current_song, play->buffered_next_song );
                    swapflt( &play->buffered_current_song_gain, &play->buffered_next_song_gain );
                    swapptr( &play->sample_pairs, &play->next_song_sample_pairs );
                    swapint( &play->sample_pairs_capacity, &play->next_song_sample_pairs_capacity );
                    swapint( &play->sample_pairs_count, &play->next_song_sample_pairs_count );
                    swapflt( &play->track_gain, &play->next_song_track_gain );
                    play->song_finished = true;
                    }
                }   
            }
        else if( !play->is_paused )
            {
            while( sample_pairs_count )
                {
                int count = play->sample_pairs_count - play->position;
                if( count > sample_pairs_count ) count = sample_pairs_count;
                for( int i = 0; i < count; ++i )
                    {
                    sample_pairs[ i * 2 + 0 ] = (APP_S16)( ( ( play->sample_pairs[ play->position * 2 + i * 2 + 0 ] ) ) * play->track_gain );
                    sample_pairs[ i * 2 + 1 ] = (APP_S16)( ( ( play->sample_pairs[ play->position * 2 + i * 2 + 1 ] ) ) * play->track_gain );
                    }
                sample_pairs += count * 2;
                sample_pairs_count -= count;
                play->position += count;        
                if( play->position >= play->sample_pairs_count )
                    {
                    play->position = 0;        
                    if( !play->loop ) 
                        {
                        swapstr( play->buffered_current_song, play->buffered_next_song );
                        swapflt( &play->buffered_current_song_gain, &play->buffered_next_song_gain );
                        swapstr( play->current_song, play->next_song );
                        swapptr( &play->sample_pairs, &play->next_song_sample_pairs );
                        swapint( &play->sample_pairs_capacity, &play->next_song_sample_pairs_capacity );
                        swapint( &play->sample_pairs_count, &play->next_song_sample_pairs_count );
                        swapflt( &play->track_gain, &play->next_song_track_gain );
                        play->song_finished = true;
                        }
                    }
                }   
            }
        else
            {
            memset( sample_pairs, 0, sample_pairs_count * 2 * sizeof( APP_S16 ) );
            }
        }
    else
        {
        memset( sample_pairs, 0, sample_pairs_count * 2 * sizeof( APP_S16 ) );
        }

    thread_mutex_unlock( &play->samples_mutex );
    }


int play_thread_proc( void* user_data )
    {
    play_thread_t* play = (play_thread_t*) user_data;

    int samples_decode_buffer_capacity = 44100 * 5; // decode 5 seconds before starting to play
    APP_S16* samples_decode_buffer = (APP_S16*) malloc( samples_decode_buffer_capacity * 2 * sizeof( APP_S16 ) );

    int next_song_sample_pairs_count = 0;
    int next_song_sample_pairs_capacity = 16 * 1024 * 1024;
    APP_S16* next_song_sample_pairs = (APP_S16*) malloc( next_song_sample_pairs_capacity * 2 * sizeof( APP_S16 ) );

    bool skip_signal = false;
    while( thread_atomic_int_load( &play->exit_flag ) == 0 )
        {
        skip_signal = false;
        thread_mutex_lock( &play->mutex );

        if( play->set_position_activated ) 
            {
            play->set_position_activated = false;
            thread_mutex_lock( &play->samples_mutex );
            play->position = (int)( play->set_position * 44100.0f );
            play->position = play->position < 0 ? 0 : play->position > play->sample_pairs_count ? play->sample_pairs_count -1 : play->position;
            thread_mutex_unlock( &play->samples_mutex );
            }

        bool new_current = false;
        if( *play->current_song == 0 ) play->sample_pairs_count = 0;
        if( *play->next_song == 0 ) play->next_song_sample_pairs_count = 0;
        if( stricmp( play->buffered_current_song, play->current_song ) != 0 ) 
            {
            play->resume_after_decode = true;
            strcpy( play->buffered_current_song, play->current_song );
            play->buffered_current_song_gain = play->current_song_gain;
            new_current = true;
            }           
        thread_mutex_unlock( &play->mutex );

        if( new_current )
            {
            if( *play->buffered_current_song )
                {
                struct stat s;
                stat( play->buffered_current_song, &s );
                mmap_t* file = mmap_open_read_only( play->buffered_current_song, s.st_size );
                if( file )
                    {
                    play->track_gain = play->buffered_current_song_gain;

                    ma_resampler* rateconv = NULL;

                    mp3dec_t dec;
                    mp3dec_init( &dec );
                    mp3dec_frame_info_t info;
                    short pcm[ MINIMP3_MAX_SAMPLES_PER_FRAME * 6 ];

                    play->is_decoding = true;
                    int copy_position = 0;
                    int samples_decode_buffer_count = 0;
                    uint8_t* ptr = (uint8_t*) mmap_data( file );
                    int size = (int) mmap_size( file );
                    while( size > 0 && thread_atomic_int_load( &play->exit_flag ) == 0 )
                        {
                        int decoded_pairs = 0;
                        while( decoded_pairs == 0 )
                            {
                            decoded_pairs = mp3dec_decode_frame( &dec, ptr, size, pcm, &info );
                            ptr += info.frame_bytes;
                            size -= info.frame_bytes;
                            if( info.frame_bytes == 0 ) break;
                            }

                        if( info.hz != 44100 ) 
                            {
                            if( !rateconv ) 
                                {
                                ma_resampler_config config = ma_resampler_config_init(
                                    ma_format_s16,
                                    info.channels,
                                    info.hz,
                                    44100,
                                    ma_resample_algorithm_linear );

                                rateconv = (ma_resampler*) malloc( sizeof( *rateconv ) );
                                ma_result result = ma_resampler_init( &config, rateconv );
                                if (result != MA_SUCCESS) 
                                    {
                                    }
                                }

                            short out_pcm[ MINIMP3_MAX_SAMPLES_PER_FRAME * 6 ];
                            ma_uint64 frameCountIn  = decoded_pairs;
                            ma_uint64 frameCountOut = MINIMP3_MAX_SAMPLES_PER_FRAME * 6;
                            ma_result result = ma_resampler_process_pcm_frames( rateconv, pcm, &frameCountIn, out_pcm, &frameCountOut );
                            if (result != MA_SUCCESS) {
                                printf( "Sample rate conversion failed (%d)\n", result );                                
                            }
                            decoded_pairs = (int)frameCountOut;
                            memcpy( pcm, out_pcm, decoded_pairs * info.channels * sizeof( short ) );
                            }
                       
                        int pairs_to_copy = decoded_pairs;
                        if( decoded_pairs > 0 )
                            {
                            if( samples_decode_buffer_count + pairs_to_copy > samples_decode_buffer_capacity ) 
                                pairs_to_copy = samples_decode_buffer_capacity - samples_decode_buffer_count;
                            if( info.channels == 2 )
                                {
                                memcpy( samples_decode_buffer + samples_decode_buffer_count * 2, pcm, pairs_to_copy * 2 * sizeof( APP_S16 ) );
                                }
                            else
                                {
                                for( int i = 0; i < pairs_to_copy; ++i )
                                    {
                                    samples_decode_buffer[ samples_decode_buffer_count * 2 + i * 2 + 0 ] = pcm[ i ];
                                    samples_decode_buffer[ samples_decode_buffer_count * 2 + i * 2 + 1 ] = pcm[ i ];
                                    }
                                }                                          
                            samples_decode_buffer_count += pairs_to_copy;
                            }

                        if( size <= 0 || info.frame_bytes == 0  || samples_decode_buffer_count >= samples_decode_buffer_capacity )
                            {
                            thread_mutex_lock( &play->samples_mutex );

                            if( copy_position == 0 ) 
                                {
                                play->position = 0;
                                play->is_paused = !play->resume_after_decode;
                                if( play->ignore_resume ) play->is_paused = true;
                                play->ignore_resume = false;
                                }

                            if( play->sample_pairs_capacity < copy_position + samples_decode_buffer_count )
                                {
                                play->sample_pairs_capacity = copy_position + samples_decode_buffer_capacity * 16;
                                play->sample_pairs = (APP_S16*) realloc( play->sample_pairs, play->sample_pairs_capacity * 2 * sizeof( APP_S16 ) );
                                }
                        
                            memcpy( play->sample_pairs + copy_position * 2, samples_decode_buffer, samples_decode_buffer_count * 2 * sizeof( APP_S16 ) );
                            copy_position += samples_decode_buffer_count;
                            play->sample_pairs_count = copy_position;

                            if( play->set_position_activated ) 
                                {
                                play->set_position_activated = false;
                                play->position = (int)( play->set_position * 44100.0f );
                                play->position = play->position < 0 ? 0 : play->position > play->sample_pairs_count ? play->sample_pairs_count -1 : play->position;
                                }
                            thread_mutex_unlock( &play->samples_mutex );

                            samples_decode_buffer_count = 0;

                            if( decoded_pairs != 0 )
                                {
                                thread_mutex_lock( &play->mutex );
                                bool current_changed = stricmp( play->buffered_current_song, play->current_song ) != 0;
                                thread_mutex_unlock( &play->mutex );
                                if( current_changed ) 
                                    {
                                    strcpy( play->buffered_current_song, "" );
                                    strcpy( play->buffered_next_song, "" );
                                    skip_signal = true;
                                    break;
                                    }
                                }
                            }
                        
                        if( size <= 0 || info.frame_bytes == 0 ) break;
                        
                        if( pairs_to_copy < decoded_pairs )
                            {
                            int offset = pairs_to_copy;
                            pairs_to_copy = decoded_pairs - pairs_to_copy;
                            if( info.channels == 2 )
                                {
                                memcpy( samples_decode_buffer + samples_decode_buffer_count * 2, pcm + offset * 2, pairs_to_copy * 2 * sizeof( APP_S16 ) );
                                }
                            else
                                {
                                for( int i = 0; i < pairs_to_copy; ++i )
                                    {
                                    samples_decode_buffer[ samples_decode_buffer_count * 2 + i * 2 + 0 ] = pcm[ i ];
                                    samples_decode_buffer[ samples_decode_buffer_count * 2 + i * 2 + 1 ] = pcm[ i ];
                                    }
                                }                                          
                            samples_decode_buffer_count += pairs_to_copy;
                            }
                        }
                    mmap_close( file );
                    play->is_decoding = false;
                    if( rateconv ) 
                        {
                        ma_resampler_uninit( rateconv );
                        free( rateconv );
                        }
                    }
                else
                    {
                    thread_mutex_lock( &play->samples_mutex );
                    play->position = 0;
                    play->sample_pairs_count = 0;
                    thread_mutex_unlock( &play->samples_mutex );
                    }
                }
            else
                {
                thread_mutex_lock( &play->samples_mutex );
                play->position = 0;
                play->sample_pairs_count = 0;
                thread_mutex_unlock( &play->samples_mutex );
                }
            }
        if( skip_signal ) continue;


        thread_mutex_lock( &play->mutex );
        bool new_next = false;       
        if( stricmp( play->buffered_next_song, play->next_song ) != 0 ) 
            {
            strcpy( play->buffered_next_song, play->next_song );
            play->buffered_next_song_gain = play->next_song_gain;
            new_next = true;
            }
        thread_mutex_unlock( &play->mutex );

        if( new_next )
            {
            if( *play->buffered_next_song )
                {
                struct stat s;
                stat( play->buffered_next_song, &s );
                mmap_t* file = mmap_open_read_only( play->buffered_next_song, s.st_size );
                if( file )
                    {
                    play->next_song_track_gain = play->buffered_next_song_gain;

                    ma_resampler* rateconv = NULL;
                    mp3dec_t dec;
                    mp3dec_init( &dec );
                    mp3dec_frame_info_t info;
                    short pcm[ MINIMP3_MAX_SAMPLES_PER_FRAME * 6 ];

                    next_song_sample_pairs_count = 0;
                    int chunk_count = 0;
                    uint8_t* ptr = (uint8_t*) mmap_data( file );
                    int size = (int) mmap_size( file );
                    while( size > 0 && thread_atomic_int_load( &play->exit_flag ) == 0 )
                        {
                        int decoded_pairs = 0;
                        while( decoded_pairs == 0 )
                            {
                            decoded_pairs = mp3dec_decode_frame( &dec, ptr, size, pcm, &info );
                            ptr += info.frame_bytes;
                            size -= info.frame_bytes;
                            if( info.frame_bytes == 0 ) break;
                            }

                        if( info.hz != 44100 ) 
                            {
                            if( !rateconv ) 
                                {
                                ma_resampler_config config = ma_resampler_config_init(
                                    ma_format_s16,
                                    info.channels,
                                    info.hz,
                                    44100,
                                    ma_resample_algorithm_linear );

                                rateconv = (ma_resampler*) malloc( sizeof( *rateconv ) );
                                ma_result result = ma_resampler_init( &config, rateconv );
                                if (result != MA_SUCCESS) 
                                    {
                                    }
                                }

                            short out_pcm[ MINIMP3_MAX_SAMPLES_PER_FRAME * 6 ];
                            ma_uint64 frameCountIn  = decoded_pairs;
                            ma_uint64 frameCountOut = MINIMP3_MAX_SAMPLES_PER_FRAME * 6;
                            ma_result result = ma_resampler_process_pcm_frames( rateconv, pcm, &frameCountIn, out_pcm, &frameCountOut );
                            if (result != MA_SUCCESS) {
                                printf( "Sample rate conversion failed (%d)\n", result );                                
                            }
                            decoded_pairs = (int)frameCountOut;
                            memcpy( pcm, out_pcm, decoded_pairs * info.channels * sizeof( short ) );
                            }

                        int pairs_to_copy = decoded_pairs;
                        if( decoded_pairs > 0 )
                            {
                            if( next_song_sample_pairs_count + pairs_to_copy > next_song_sample_pairs_capacity ) 
                                {
                                next_song_sample_pairs_capacity += samples_decode_buffer_capacity * 16;
                                next_song_sample_pairs = (APP_S16*) realloc( next_song_sample_pairs, next_song_sample_pairs_capacity * 2 * sizeof( APP_S16 ) );
                                }
                            if( info.channels == 2 )
                                {
                                memcpy( next_song_sample_pairs + next_song_sample_pairs_count * 2, pcm, pairs_to_copy * 2 * sizeof( APP_S16 ) );
                                }
                            else
                                {
                                for( int i = 0; i < pairs_to_copy; ++i )
                                    {
                                    next_song_sample_pairs[ next_song_sample_pairs_count * 2 + i * 2 + 0 ] = pcm[ i ];
                                    next_song_sample_pairs[ next_song_sample_pairs_count * 2 + i * 2 + 1 ] = pcm[ i ];
                                    }
                                }                                          
                            next_song_sample_pairs_count += pairs_to_copy;
                            chunk_count += pairs_to_copy;
                            }

                        if( size <= 0 || info.frame_bytes == 0 ) break;                       
                        thread_yield();

                        if( play->set_position_activated ) 
                            {
                            play->set_position_activated = false;
                            thread_mutex_lock( &play->samples_mutex );
                            play->position = (int)( play->set_position * 44100.0f );
                            play->position = play->position < 0 ? 0 : play->position > play->sample_pairs_count ? play->sample_pairs_count -1 : play->position;
                            thread_mutex_unlock( &play->samples_mutex );
                            }

                        if( chunk_count >= samples_decode_buffer_capacity )
                            {
                            chunk_count = 0;
                            thread_mutex_lock( &play->mutex );
                            bool current_changed = stricmp( play->buffered_current_song, play->current_song ) != 0;
                            bool next_changed = stricmp( play->buffered_next_song, play->next_song ) != 0;
                            thread_mutex_unlock( &play->mutex );
                            if( current_changed || next_changed ) 
                                {
                                strcpy( play->buffered_current_song, "" );
                                strcpy( play->buffered_next_song, "" );
                                new_current = false;
                                new_next = false;
                                skip_signal = true;
                                break;
                                }
                            }
                        
                        }
                    mmap_close( file );
                    if( rateconv ) 
                        {
                        ma_resampler_uninit( rateconv );
                        free( rateconv );
                        }

                    thread_mutex_lock( &play->samples_mutex );
                    swapptr( &next_song_sample_pairs, &play->next_song_sample_pairs );
                    swapint( &next_song_sample_pairs_capacity, &play->next_song_sample_pairs_capacity );
                    swapint( &next_song_sample_pairs_count, &play->next_song_sample_pairs_count );
                    thread_mutex_unlock( &play->samples_mutex );
                    }
                 else
                    {
                    thread_mutex_lock( &play->samples_mutex );
                    play->next_song_sample_pairs_count = 0;
                    thread_mutex_unlock( &play->samples_mutex );
                    }
                }
            else
                {
                thread_mutex_lock( &play->samples_mutex );
                play->next_song_sample_pairs_count = 0;
                thread_mutex_unlock( &play->samples_mutex );
                }
           }
        if( skip_signal ) continue;

        thread_signal_wait( &play->signal, 1000 );
        }

    if( next_song_sample_pairs ) free( next_song_sample_pairs );
    free( samples_decode_buffer );  
    return 0;
    }


play_thread_t* play_thread_create( void )
    {
    play_thread_t* play = (play_thread_t*) malloc( sizeof( play_thread_t ) );
    memset( play, 0, sizeof( *play ) );
    
    play->ffwd_speed = g_ffwd_speed;
    play->rewind_speed = g_rewind_speed;

    thread_mutex_init( &play->mutex );
    thread_signal_init( &play->signal );
    thread_atomic_int_store( &play->exit_flag, 0 );
    thread_mutex_init( &play->samples_mutex );

    play->sample_pairs_capacity = 16 * 1024 * 1024;
    play->sample_pairs = (APP_S16*) malloc( play->sample_pairs_capacity * 2 * sizeof( APP_S16 ) );

    play->next_song_sample_pairs_capacity = 16 * 1024 * 1024;
    play->next_song_sample_pairs = (APP_S16*) malloc( play->next_song_sample_pairs_capacity * 2 * sizeof( APP_S16 ) );

    play->thread_handle = thread_create( play_thread_proc, play, 0, THREAD_STACK_SIZE_DEFAULT );
    return play;
    }


void play_thread_destroy( play_thread_t* play )
    {
    thread_atomic_int_store( &play->exit_flag, 1 );
    thread_signal_raise( &play->signal );
    thread_join( play->thread_handle );
    
    thread_signal_term( &play->signal );
    thread_mutex_term( &play->mutex );
    if( play->sample_pairs ) free( play->sample_pairs );
    if( play->next_song_sample_pairs ) free( play->next_song_sample_pairs );
    free( play );
    }


void play_thread_pause( play_thread_t* play, bool is_paused )
    {
    thread_mutex_lock( &play->samples_mutex );
    play->is_paused = is_paused;
    thread_mutex_unlock( &play->samples_mutex );
    }


void play_thread_ffwd( play_thread_t* play, bool is_ffwd )
    {
    thread_mutex_lock( &play->samples_mutex );
    play->is_ffwd = is_ffwd;
    thread_mutex_unlock( &play->samples_mutex );
    }


void play_thread_rewind( play_thread_t* play, bool is_rewind )
    {
    thread_mutex_lock( &play->samples_mutex );
    play->is_rewind = is_rewind;
    thread_mutex_unlock( &play->samples_mutex );
    }


void play_thread_change_song( play_thread_t* play, char const* song, float song_gain, char const* next_song, float next_song_gain, float position )
    {
    thread_mutex_lock( &play->samples_mutex );
    play->is_paused = true;
    thread_mutex_unlock( &play->samples_mutex );

    thread_mutex_lock( &play->mutex );

    strcpy( play->current_song, song );
    play->current_song_gain = song_gain;
    strcpy( play->next_song, next_song ? next_song : "" );
    play->next_song_gain = next_song_gain;
    play->set_position = position;
    play->song_finished = false;

    thread_signal_raise( &play->signal );
    thread_mutex_unlock( &play->mutex );
    }


bool play_thread_song_finished( play_thread_t* play )
    {
    thread_mutex_lock( &play->mutex );

    bool value = play->song_finished;

    thread_mutex_unlock( &play->mutex );
    return value;
    }


bool play_thread_queue_song( play_thread_t* play, char const* next_song, float next_song_gain )
    {
    thread_mutex_lock( &play->mutex );

    strcpy( play->next_song, next_song );
    play->next_song_gain = next_song_gain;
    play->song_finished = false;

    thread_signal_raise( &play->signal );
    thread_mutex_unlock( &play->mutex );
    return false;
    }


void play_thread_set_position( play_thread_t* play, float position )
    {
    thread_mutex_lock( &play->mutex );

    play->set_position_activated = true;
    play->set_position = position;
    play->song_finished = false;

    thread_signal_raise( &play->signal );
    thread_mutex_unlock( &play->mutex );
    }


float play_thread_get_position( play_thread_t* play )
    {
    thread_mutex_lock( &play->mutex );
    bool changing = stricmp( play->buffered_current_song, play->current_song ) != 0;
    thread_mutex_unlock( &play->mutex );
    if( changing ) return 0.0f;

    thread_mutex_lock( &play->samples_mutex );

    int position = play->position;
    
    thread_mutex_unlock( &play->samples_mutex );

    return position / 44100.0f;
    }


float play_thread_get_length( play_thread_t* play )
    {
    thread_mutex_lock( &play->mutex );
    bool changing = stricmp( play->buffered_current_song, play->current_song ) != 0;
    thread_mutex_unlock( &play->mutex );
    if( changing ) return 0.0f;

    thread_mutex_lock( &play->samples_mutex );

    int length = play->sample_pairs_count;
    
    thread_mutex_unlock( &play->samples_mutex );

    return length / 44100.0f;
    }

void play_thread_loop( play_thread_t* play, bool loop )
    {
    thread_mutex_lock( &play->mutex );
    play->loop = loop;
    thread_mutex_unlock( &play->mutex );
    }

