
typedef struct play_control_t
    {
    play_thread_t* play_thread;

    uint32_t album_id;

    musicdb_song_t tracks[ 256 * 1024 ];
    int tracks_shuffle[ 256 * 1024 ];
    int tracks_count;
    int track_index;

    bool is_paused;
    bool is_ffwd;
    bool is_rewind;

    bool loop;
    bool shuffle;
    bool repeat;
    } play_control_t;

void play_control_pause( play_control_t* control );

void play_control_init( play_control_t* control, play_thread_t* play_thread )
    {
    memset( control, 0, sizeof( *control ) );
    control->play_thread = play_thread;
    control->album_id = MUSICDB_INVALID_ID;
    }


bool play_control_update( play_control_t* control )
    {
    if( control->tracks_count == 0 ) return false;
    if( play_thread_song_finished( control->play_thread ) )
        {
        if( control->loop ) return true;
        control->track_index++;
        if( control->track_index >= control->tracks_count ) 
            {
            control->track_index = 0;
            if( !( control->repeat || control->album_id == MUSICDB_INVALID_ID ) ) 
                {   
                char const* filename = control->shuffle ? control->tracks[ control->tracks_shuffle[ control->track_index ] ].filename : control->tracks[ control->track_index ].filename;
                play_control_pause( control );
                play_thread_change_song( control->play_thread, filename, 
                    "", 0.0f );
                for( int i = 0; i < 1000; ++i ) 
                    {
                    thread_yield();
                    bool change_done;
                    thread_mutex_lock( &control->play_thread->mutex );
                    change_done = strcmp( control->play_thread->buffered_current_song, control->play_thread->current_song ) == 0;
                    thread_mutex_unlock( &control->play_thread->mutex );
                    if( change_done ) 
                        break;
                    sleep( 10 );
                    }
                control->play_thread->resume_after_decode = false;
                play_control_pause( control );
                }
            }

        int next_index = control->track_index + 1;
        if( next_index >= control->tracks_count ) next_index = 0;   
        char const* filename = control->shuffle ? control->tracks[ control->tracks_shuffle[ next_index ] ].filename : control->tracks[ next_index ].filename;
        play_thread_queue_song( control->play_thread, ( control->repeat || control->album_id == MUSICDB_INVALID_ID ) || next_index != 0 ? filename : "" );
        return true;
        }

    return false;
    }


void play_control_loop( play_control_t* control, bool loop )
    {
    control->loop = loop;
    play_thread_loop( control->play_thread, loop );
    }


void play_control_repeat( play_control_t* control, bool repeat )
    {
    control->repeat = repeat;
    int next_index = control->track_index + 1;
    if( next_index >= control->tracks_count ) next_index = 0;
    if( next_index == 0 )
        play_thread_queue_song( control->play_thread, repeat ? control->tracks[ next_index ].filename : "" );
    }


void play_control_shuffle( play_control_t* control, bool shuffle )
    {
    control->shuffle = shuffle;
    if( control->shuffle )
        {
        for( int i = 0; i < control->tracks_count; ++i )
            {
            if( control->tracks_shuffle[ i ] == control->track_index )
                {
                control->track_index = i;
                break;
                }
            }
        }
    else
        {
        control->track_index = control->tracks_shuffle[ control->track_index ]; 
        }
    }


void play_control_album( play_control_t* control, uint32_t album_id, musicdb_song_t* tracks, int tracks_count )
    {
    if( control->album_id == album_id ) return;

    control->album_id = album_id;
    if( control->album_id == MUSICDB_INVALID_ID ) 
        {
        play_thread_change_song( control->play_thread, "", "", 0.0f );
        control->tracks_count = 0;
        control->track_index = -1;
        return;
        }

    for( int i = 0; i < tracks_count; ++i ) 
        { 
        control->tracks[ i ] = tracks[ i ];
        }
    control->tracks_count = tracks_count;
    control->track_index = 0;

    for( int i = 0; i < control->tracks_count; ++i ) 
        { 
        control->tracks_shuffle[ i ] = i;
        }
    shuffle_ints( control->tracks_shuffle, control->tracks_count );
    shuffle_ints( control->tracks_shuffle, control->tracks_count );
    

    int next_index = control->track_index + 1;
    if( next_index >= control->tracks_count && control->repeat ) next_index = 0;

    char const* filename = control->track_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ control->track_index ] ].filename : control->tracks[ control->track_index ].filename ): NULL;
    char const* filename_next = next_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ next_index ] ].filename : control->tracks[ next_index ].filename ) : NULL;

    play_thread_change_song( control->play_thread, filename, filename_next, 0.0f );
    control->is_paused = false;
    }


void play_control_track( play_control_t* control, int track_index )
    {
    if( control->tracks_count == 0 ) return;
    if( track_index < 0 || track_index >= control->tracks_count ) return;

    if( control->shuffle )
        {
        for( int i = 0; i < control->tracks_count; ++i )
            {
            if( control->tracks_shuffle[ i ] == track_index )
                {
                track_index = i;
                break;
                }
            }
        }

    if( control->track_index == track_index )
        {
        if( control->is_paused )
            {
            play_thread_pause( control->play_thread, false );
            control->is_paused = false;
            }
        return;
        }
    control->track_index = track_index;

    int next_index = control->track_index + 1;
    if( next_index >= control->tracks_count ) next_index = 0;

    char const* filename = control->track_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ control->track_index ] ].filename : control->tracks[ control->track_index ].filename ): NULL;
    char const* filename_next = next_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ next_index ] ].filename : control->tracks[ next_index ].filename ) : NULL;
    play_thread_change_song( control->play_thread, filename, filename_next, 0.0f );

    control->is_paused = false;
    }


void play_control_shuffle_list( play_control_t* control, musicdb_song_t* tracks, int tracks_count ) 
{
    control->album_id = MUSICDB_INVALID_ID;

    for( int i = 0; i < tracks_count; ++i ) 
        { 
        control->tracks[ i ] = tracks[ i ];
        }
    control->tracks_count = tracks_count;
    control->track_index = 0;

    for( int i = 0; i < control->tracks_count; ++i ) 
        { 
        control->tracks_shuffle[ i ] = i;
        }
   
    int next_index = control->track_index + 1;
    if( next_index >= control->tracks_count ) next_index = 0;

    char const* filename = control->track_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ control->track_index ] ].filename : control->tracks[ control->track_index ].filename ): NULL;
    char const* filename_next = next_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ next_index ] ].filename : control->tracks[ next_index ].filename ) : NULL;

    play_thread_change_song( control->play_thread, filename, filename_next, 0.0f );
    control->is_paused = false;
}



void play_control_pause( play_control_t* control )
    {
    if( control->tracks_count == 0 ) return;
    play_thread_pause( control->play_thread, true );
    control->is_paused = true;
    }


void play_control_stop( play_control_t* control )
    {
    if( control->tracks_count == 0 ) return;
    play_thread_set_position( control->play_thread, 0.0f );
    play_thread_pause( control->play_thread, true );
    control->is_paused = true;
    }


void play_control_ffwd( play_control_t* control, bool is_ffwd )
    {
    if( control->tracks_count == 0 ) return;
    play_thread_ffwd( control->play_thread, is_ffwd );
    control->is_ffwd = is_ffwd;
    }


void play_control_rewind( play_control_t* control, bool is_rewind )
    {
    if( control->tracks_count == 0 ) return;
    play_thread_rewind( control->play_thread, is_rewind );
    control->is_rewind = is_rewind;
    }


void play_control_play( play_control_t* control )
    {
    if( control->tracks_count == 0 ) return;
    if( control->is_paused )
        {
        play_thread_pause( control->play_thread, false );
        control->is_paused = false;
        }
    else
        {
        int next_index = control->track_index + 1;
        if( next_index >= control->tracks_count && ( control->repeat || control->album_id == MUSICDB_INVALID_ID ) ) next_index = 0;

        char const* filename = control->track_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ control->track_index ] ].filename : control->tracks[ control->track_index ].filename ): NULL;
        char const* filename_next = next_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ next_index ] ].filename : control->tracks[ next_index ].filename ) : NULL;
        play_thread_change_song( control->play_thread, filename, filename_next, 0.0f );
        control->is_paused = false;
        }
    }


void play_control_next( play_control_t* control )
    {
    if( control->tracks_count == 0 ) return;
    if( control->tracks_count == 0 ) return;

    control->track_index++;
    if( control->track_index >= control->tracks_count ) {
        control->track_index = 0;
        shuffle_ints( control->tracks_shuffle, control->tracks_count );
    }
    
    int next_index = control->track_index + 1;
    if( next_index >= control->tracks_count ) next_index = 0;
    
    char const* filename = control->track_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ control->track_index ] ].filename : control->tracks[ control->track_index ].filename ): NULL;
    char const* filename_next = next_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ next_index ] ].filename : control->tracks[ next_index ].filename ) : NULL;
    play_thread_change_song( control->play_thread, filename, filename_next, 0.0f );
    control->is_paused = false;
    }


void play_control_prev( play_control_t* control )
    {
    if( control->tracks_count == 0 ) return;
    if( control->tracks_count == 0 ) return;

    control->track_index--;
    if( control->track_index < 0 ) control->track_index = control->tracks_count - 1;
    
    int next_index = control->track_index + 1;
    if( next_index >= control->tracks_count ) next_index = 0;
    
    char const* filename = control->track_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ control->track_index ] ].filename : control->tracks[ control->track_index ].filename ): NULL;
    char const* filename_next = next_index < control->tracks_count ? ( control->shuffle ? control->tracks[ control->tracks_shuffle[ next_index ] ].filename : control->tracks[ next_index ].filename ) : NULL;
    play_thread_change_song( control->play_thread, filename, filename_next, 0.0f );
    control->is_paused = false;
    }


int play_control_track_index( play_control_t* control )
    {
    int track_index = control->track_index;
    track_index = track_index < 0 ? 0 : track_index >= control->tracks_count ? control->tracks_count - 1 : track_index;
    return control->shuffle ? control->tracks_shuffle[ track_index ] : track_index; 
    }


float play_control_length( play_control_t* control )
    {
    if( control->play_thread->is_decoding ) return 0;
    if( control->tracks_count == 0 ) return 0;
    int track_index = control->track_index;
    if( track_index < 0 || track_index >= control->tracks_count ) return 0;
    return play_thread_get_length( control->play_thread );
    }


float play_control_position( play_control_t* control )
    {
    if( control->tracks_count == 0 ) return 0;
    int track_index = control->track_index;
    if( track_index < 0 || track_index >= control->tracks_count ) return 0;
    return play_thread_get_position( control->play_thread );
    }


void play_control_position_set( play_control_t* control, float position )
    {
    if( control->tracks_count == 0 ) return;
    int track_index = control->track_index;
    if( track_index < 0 || track_index >= control->tracks_count ) return;
    play_thread_set_position( control->play_thread, position );
    }
