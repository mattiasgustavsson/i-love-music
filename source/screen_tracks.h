
typedef struct track_t
    {
    text_t* text_title;
    text_t* text_tracknum;
    text_t* text_tracklen;
    text_t* text_track_artist;
    } track_t;   


typedef struct pic_t
    {
    uint64_t hash;
    int pic_type;
    bitmap_t* bmp;
    } pic_t;

typedef struct screen_tracks_t
    {
    uint64_t cover_hash;
    app_t* app;
    render_t* render;
    int width;
    int height;


    int scroll;
    int layout_max_scroll;

    bitmap_t* circle;
    bitmap_t* all_discs;

    play_control_t* play_control;

    uint32_t album_id;
    musicdb_thumbnail_t album_thumb;
    int cover_delay;

    slider_bitmaps_t slider_bitmaps;

    font_t* font_artist;
    font_t* font_album;
    font_t* font_track_artist;
    font_t* font_year;
    font_t* font_title;
    font_t* font_length;
    font_t* font_slider;

    bitmap_t* frame;

    text_t* text_artist;
    text_t* text_album;
    text_t* text_year;
    bitmap_t* cover;

    music_db_t* db;

    button_t ctl_prev;
    button_t ctl_rewind;
    button_t ctl_pause;
    button_t ctl_stop;
    button_t ctl_ffwd;
    button_t ctl_next;

    button_t ctl_loop;
    button_t ctl_shuffle;
    button_t ctl_repeat;

    bitmap_t* bmp_play;
    bitmap_t* bmp_pause;

    text_t* text_elapsed;
    text_t* text_remaining;
    slider_t time_slider;
    int prev_pos;

    bool is_ffwd;
    bool is_rewind;

    bool big_pic;
    bool big_redraw;
    float zoom;
    float target_zoom;
    float zoom_time;
    int big_x;
    int big_y;
    int big_w;
    int big_h;

    bool cover_hover;
    bool hover_artist;
    int hover_track;
    int hover_track_width;
    int hover_track_y;
    int hover_track_h;

    musicdb_album_t album_info;
 
    track_t tracks[ 256 ];   
    musicdb_song_t song_info[ 256 ];
    int tracks_count;
    bool various;

    pic_t pics[ 256 ];
    int pics_count;

    int pics_extract_track;
    int pics_extract_index;
    id3tag_t* pics_extract_tag;

    int pic_hover;
    int pic_hover_x;
    int pic_hover_y;
    int selected_pic;

    int discs_count;
    int discs[ 32 ];
    text_t* disc_num[ 32 ];
    int selected_disc;
    int hover_disc;
    bool hover_discs;
    } screen_tracks_t;


bool screen_tracks_init( screen_tracks_t* screen, app_t* app, render_t* render, play_control_t* play_control, bitmap_t* frame, slider_bitmaps_t slider_bitmaps )
    {
    memset( screen, 0, sizeof( *screen ) );
    screen->app = app;
    screen->render = render;
    screen->width = (int)render->logical_width;
    screen->height = (int)render->logical_height;
    screen->frame = frame;
    screen->slider_bitmaps = slider_bitmaps;
    screen->play_control = play_control;

    screen->album_id = MUSICDB_INVALID_ID;

    screen->circle = load_app_bitmap( render, "gui/icons/disc.png", ".cache/gui/icons/disc.mip", global_max_w, global_max_h );
    if( !screen->circle ) app_fatal_error( app, "Failed to load texture" );

    screen->all_discs = load_app_bitmap( render, "gui/icons/discs.png", ".cache/gui/icons/discs.mip", global_max_w, global_max_h );
    if( !screen->all_discs ) app_fatal_error( app, "Failed to load texture" );


    screen->font_artist = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 38 );
    if( !screen->font_artist ) app_fatal_error( app, "Failed to load font" );
        
    screen->font_album = font_create( screen->render, "gui/fonts/AlegreyaSans-Bold.ttf", 33 );
    if( !screen->font_album ) app_fatal_error( app, "Failed to load font" );

    screen->font_track_artist = font_create( screen->render, "gui/fonts/AlegreyaSans-MediumItalic.ttf", 21 );
    if( !screen->font_track_artist ) app_fatal_error( app, "Failed to load font" );
        
    screen->font_year = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 24 );
    if( !screen->font_year ) app_fatal_error( app, "Failed to load font" );

    screen->font_title = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 28 );
    if( !screen->font_title ) app_fatal_error( app, "Failed to load font" );

    screen->font_length = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 21 );
    if( !screen->font_length ) app_fatal_error( app, "Failed to load font" );

    screen->font_slider = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 24 );
    if( !screen->font_slider ) app_fatal_error( app, "Failed to load font" );

    int x = 84;
    int y = 715;

    button_init( &screen->ctl_prev, render, load_app_bitmap( render, "gui/icons/prev.png", ".cache/gui/icons/prev.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_prev, x, y, 64, 64 );
    button_modify_bounds( &screen->ctl_prev, 7, 7, -7, -7 );
    x += 64;

    button_init( &screen->ctl_rewind, render, load_app_bitmap( render, "gui/icons/rewind.png", ".cache/gui/icons/rewind.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_rewind, x, y, 64, 64 );
    button_modify_bounds( &screen->ctl_rewind, 7, 7, -7, -7 );
    x += 64;

    button_init( &screen->ctl_pause, render, load_app_bitmap( render, "gui/icons/pause.png", ".cache/gui/icons/pause.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_pause, x, y, 64, 64 );
    button_modify_bounds( &screen->ctl_pause, 7, 7, -7, -7 );
    x += 64;

    button_init( &screen->ctl_stop, render, load_app_bitmap( render, "gui/icons/stop.png", ".cache/gui/icons/stop.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_stop, x, y, 64, 64 );
    button_modify_bounds( &screen->ctl_stop, 7, 7, -7, -7 );
    x += 64;

    button_init( &screen->ctl_ffwd, render, load_app_bitmap( render, "gui/icons/ffwd.png", ".cache/gui/icons/ffwd.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_ffwd, x, y, 64, 64 );
    button_modify_bounds( &screen->ctl_ffwd, 7, 7, -7, -7 );
    x += 64;

    button_init( &screen->ctl_next, render, load_app_bitmap( render, "gui/icons/next.png", ".cache/gui/icons/next.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_next, x, y, 64, 64 );
    button_modify_bounds( &screen->ctl_next, 7, 7, -7, -7 );
    x += 64;

    x = 100 + 600 + 4;
    y = 729;

    x -= 42;
    button_init( &screen->ctl_repeat, render, load_app_bitmap( render, "gui/icons/repeat.png", ".cache/gui/icons/repeat.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_repeat, x, y, 35, 35 );

    x -= 42;
    button_init( &screen->ctl_shuffle, render, load_app_bitmap( render, "gui/icons/shuffle.png", ".cache/gui/icons/shuffle.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_shuffle, x, y, 35, 35 );

    x -= 42;
    button_init( &screen->ctl_loop, render, load_app_bitmap( render, "gui/icons/loop.png", ".cache/gui/icons/loop.mip", global_max_w, global_max_h ) );
    button_size( &screen->ctl_loop, x, y, 35, 35 );

    screen->bmp_play = load_app_bitmap( render, "gui/icons/play.png", ".cache/gui/icons/play.mip", global_max_w, global_max_h ) ;
    screen->bmp_pause = screen->ctl_pause.bmp;

    slider_init( &screen->time_slider, render, slider_bitmaps, 150, 780, 600 - 100, 16 );
    return true;
    }


void screen_tracks_term( screen_tracks_t* screen )
    {
    bitmap_destroy( screen->render, screen->circle );
    bitmap_destroy( screen->render, screen->all_discs );
    bitmap_destroy( screen->render, screen->bmp_play );
    bitmap_destroy( screen->render, screen->bmp_pause );
    screen->ctl_pause.bmp = 0;
    button_term( &screen->ctl_prev );
    button_term( &screen->ctl_rewind );
    button_term( &screen->ctl_pause );
    button_term( &screen->ctl_stop );
    button_term( &screen->ctl_ffwd );
    button_term( &screen->ctl_next );
    button_term( &screen->ctl_repeat );
    button_term( &screen->ctl_shuffle );
    button_term( &screen->ctl_loop );

    for( int i = 0; i < screen->discs_count; ++i )
        if( screen->disc_num[ i ] ) text_destroy( screen->render, screen->disc_num[ i ] );

    if( screen->pics_extract_tag ) id3tag_free( screen->pics_extract_tag );
    for( int i = 0; i < screen->pics_count; ++i )
        if( screen->pics[ i ].bmp != screen->cover) bitmap_destroy( screen->render, screen->pics[ i ].bmp );

    if( screen->cover ) bitmap_destroy( screen->render, screen->cover );

    if( screen->text_artist ) text_destroy( screen->render, screen->text_artist );
    if( screen->text_album ) text_destroy( screen->render, screen->text_album );
    if( screen->text_year ) text_destroy( screen->render, screen->text_year );
    for( int i = 0; i < screen->tracks_count; ++i )
        {
        track_t* track = &screen->tracks[ i ];
        if( track->text_title ) text_destroy( screen->render, track->text_title );
        if( track->text_tracknum ) text_destroy( screen->render, track->text_tracknum );
        if( track->text_tracklen ) text_destroy( screen->render, track->text_tracklen );
        if( track->text_track_artist ) text_destroy( screen->render, track->text_track_artist );
        }
    if( screen->text_elapsed ) text_destroy( screen->render, screen->text_elapsed );
    if( screen->text_remaining ) text_destroy( screen->render, screen->text_remaining    );

    if( screen->font_track_artist ) font_destroy( screen->render, screen->font_track_artist );
    if( screen->font_artist ) font_destroy( screen->render, screen->font_artist );
    if( screen->font_album ) font_destroy( screen->render, screen->font_album );
    if( screen->font_year ) font_destroy( screen->render, screen->font_year );
    if( screen->font_title ) font_destroy( screen->render, screen->font_title );
    if( screen->font_length ) font_destroy( screen->render, screen->font_length );
    if( screen->font_slider ) font_destroy( screen->render, screen->font_slider );
    }


void screen_tracks_set_album( screen_tracks_t* screen, uint32_t album_id )
    {
    if( screen->album_id != album_id )
        {
        screen->scroll = 0;
        screen->album_id = album_id;

        if( screen->pics_extract_tag ) id3tag_free( screen->pics_extract_tag );
        screen->pics_extract_tag = 0;
        for( int i = 0; i < screen->pics_count; ++i )
            if( screen->pics[ i ].bmp != screen->cover) bitmap_destroy( screen->render, screen->pics[ i ].bmp );
        screen->pics_count = 0;
        screen->pics_extract_track = 0;
        screen->pics_extract_index = 0;
        screen->pic_hover = -1;
        screen->selected_pic =  -1;

        if( screen->cover ) bitmap_destroy( screen->render, screen->cover );
        if( screen->text_artist ) text_destroy( screen->render, screen->text_artist );
        if( screen->text_album ) text_destroy( screen->render, screen->text_album );
        if( screen->text_year ) text_destroy( screen->render, screen->text_year );
        for( int i = 0; i < screen->tracks_count; ++i )
            {
            track_t* track = &screen->tracks[ i ];
            if( track->text_title ) text_destroy( screen->render, track->text_title );
            if( track->text_tracknum ) text_destroy( screen->render, track->text_tracknum );
            if( track->text_tracklen ) text_destroy( screen->render, track->text_tracklen );
            if( track->text_track_artist ) text_destroy( screen->render, track->text_track_artist );
            }
        for( int i = 0; i < screen->discs_count; ++i )
            {
            if( screen->disc_num[ i ] ) text_destroy( screen->render, screen->disc_num[ i ] );
            screen->disc_num[ i ] = 0;
            }

        screen->cover = 0;
        screen->text_artist = 0;
        screen->text_album = 0;
        screen->text_year = 0;
        screen->tracks_count = 0;
        screen->big_pic = false;
        screen->zoom = 1.0f;
        screen->target_zoom = 1.0f;
        screen->zoom_time = 0.0f;
        screen->discs_count = 0;
        screen->selected_disc = -1;
        screen->hover_disc = -1;
        screen->hover_discs = false;
        }
    }


int intcmp( void const* a, void const* b ) 
    {
    int const* ia = (int const*)a;
    int const* ib = (int const*)b;
    if( *ia > *ib ) 
        {
        return 1;
        } 
    else if( *ia < *ib ) 
        {
        return -1;
        }
    return 0;
    }


void screen_tracks_refresh( screen_tracks_t* screen, music_db_t* musicdb )
    {
    screen->db = musicdb;
    musicdb_album_t album_info;    
    int songs_count = 0;
    musicdb_song_t* songs = musicdb_songs_get( musicdb, screen->album_id, &songs_count, &album_info, &screen->album_thumb );   
    if( !songs ) {
        if( screen->play_control->album_id == screen->album_id ) 
            {
            play_control_album( screen->play_control, MUSICDB_INVALID_ID, NULL, 0 );
            }
        screen_tracks_set_album( screen, MUSICDB_INVALID_ID );
        songs = musicdb_songs_get( musicdb, screen->album_id, &songs_count, &album_info, &screen->album_thumb );   
        if( !songs ) return;
    }

    if( songs_count != screen->tracks_count )
        {
        if( screen->pics_extract_tag ) id3tag_free( screen->pics_extract_tag );
        screen->pics_extract_tag = 0;
        for( int i = 0; i < screen->pics_count; ++i )
            if( screen->pics[ i ].bmp != screen->cover) bitmap_destroy( screen->render, screen->pics[ i ].bmp );
        screen->pics_count = 0;
        screen->pics_extract_track = 0;
        screen->pics_extract_index = 0;
        screen->pic_hover = -1;
        screen->selected_pic =  -1;

        if( screen->cover ) bitmap_destroy( screen->render, screen->cover );
        if( screen->text_artist ) text_destroy( screen->render, screen->text_artist );
        if( screen->text_album ) text_destroy( screen->render, screen->text_album );
        if( screen->text_year ) text_destroy( screen->render, screen->text_year );
        for( int i = 0; i < screen->tracks_count; ++i )
            {
            track_t* track = &screen->tracks[ i ];
            if( track->text_title ) text_destroy( screen->render, track->text_title );
            if( track->text_tracknum ) text_destroy( screen->render, track->text_tracknum );
            if( track->text_tracklen ) text_destroy( screen->render, track->text_tracklen );
            if( track->text_track_artist ) text_destroy( screen->render, track->text_track_artist );
            }
        for( int i = 0; i < screen->discs_count; ++i )
            {
            if( screen->disc_num[ i ] ) 
                {
                text_destroy( screen->render, screen->disc_num[ i ] );
                screen->disc_num[ i ] = 0;
                }
            }
        screen->cover = 0;
        screen->text_artist = 0;
        screen->text_album = 0;
        screen->text_year = 0;
        screen->tracks_count = 0;

        screen->discs_count = 0;
        screen->tracks_count = songs_count;
        for( int i = 0; i < songs_count; ++i )
            {
            track_t* track = &screen->tracks[ i ];
            memset( track, 0, sizeof( *track ) );
            screen->song_info[ i ] = songs[ i ];
            int disc = songs[ i ].disc_number;
            if( disc <= 0 ) disc = 1;
            for( int j = 0; j < screen->discs_count; ++j )
                {
                if( screen->discs[ j ] == disc ) 
                    {
                    disc = -1;
                    break;
                    }
                }
            if( disc > 0 ) screen->discs[ screen->discs_count++ ] = disc;
            }
        memset( screen->disc_num, 0, sizeof( screen->disc_num ) );
        qsort( screen->discs, screen->discs_count, sizeof( *screen->discs ), intcmp );
        screen->selected_disc = -1;
        screen->hover_disc = -1;
        screen->hover_discs = false;
        screen->cover_delay = 2;

        int w, h, mip_width, mip_height;
        bitmap_t* thumb = musicdb_get_album_thumbnail( musicdb, album_info.id, &w, &h, &mip_width, &mip_height, screen->render );
        if( thumb )
            {
            screen->cover = thumb;
            }
        }


    screen->album_info = album_info;
    screen->various = album_info.various;
    if( !screen->various ) 
        {
        for( int i = 0; i < screen->tracks_count; ++i )
            {
            if( strcmp( screen->song_info[ i ].artist, screen->album_info.artist ) != 0 )
                {
                bool all_same = true;
                for( int j = 1; j < screen->tracks_count; ++j )
                    {
                    if( strcmp( screen->song_info[ j ].artist, screen->song_info[ 0 ].artist ) != 0 )
                        {
                        all_same = false;
                        break;
                        }
                    }
                if( !all_same )
                    screen->various = true;
                break;
                }
            }
        }
    musicdb_songs_release( musicdb, songs );

    }


typedef struct screen_tracks_navigation_t
    {
    screen_t screen;
    uint32_t artist_id;
    } screen_tracks_navigation_t;


bool screen_tracks_update( screen_tracks_t* screen, screen_tracks_navigation_t* navigation, bool resize, input_t const* orig_input )
    {
    (void) screen, navigation, resize;
    navigation->screen = SCREEN_TRACKS;
    navigation->artist_id = MUSICDB_INVALID_ID;

    input_t input_copy = *orig_input;
    if( input_copy.mouse_y > 80 )
        {
        input_copy.mouse_y += (int) screen->scroll;
        input_copy.click_start_y += (int) screen->scroll;
        }
    input_t* input = &input_copy;

    bool redraw = false;
    int prev_track_index = play_control_track_index( screen->play_control );

    if( resize || screen->width != (int)screen->render->logical_width || screen->height != (int)screen->render->logical_height )
        {

        screen->width = (int)screen->render->logical_width;
        screen->height = (int)screen->render->logical_height;

        if( screen->text_artist ) text_destroy( screen->render, screen->text_artist );
        if( screen->text_album ) text_destroy( screen->render, screen->text_album );
        if( screen->text_year ) text_destroy( screen->render, screen->text_year );
        screen->text_artist = 0;
        screen->text_album = 0;
        screen->text_year = 0;

        for( int i = 0; i < screen->discs_count; ++i )
            {
            if( screen->disc_num[ i ] ) 
                {
                text_destroy( screen->render, screen->disc_num[ i ] );
                screen->disc_num[ i ] = 0;
                }
            }


        for( int i = 0; i < screen->tracks_count; ++i )
            {
            track_t* track = &screen->tracks[ i ];
            if( track->text_title ) text_destroy( screen->render, track->text_title );
            track->text_title = 0;
            if( track->text_tracknum ) text_destroy( screen->render, track->text_tracknum );
            track->text_tracknum = 0;            
            if( track->text_tracklen ) text_destroy( screen->render, track->text_tracklen );
            track->text_tracklen = 0;            
            if( track->text_track_artist ) text_destroy( screen->render, track->text_track_artist );
            track->text_track_artist = 0;            
            }

        char str[ 16 ];
        strcpy( str,"0:00" );
        if( screen->text_elapsed ) 
            {
            strcpy( str, screen->text_elapsed->string );
            text_destroy( screen->render, screen->text_elapsed );
            }
        screen->text_elapsed = text_create( screen->render, screen->font_slider, str );    
        
        strcpy( str,"0:00" );
        if( screen->text_remaining ) 
            {
            strcpy( str, screen->text_elapsed->string );
            text_destroy( screen->render, screen->text_remaining );
            }
        screen->text_remaining = text_create( screen->render, screen->font_slider, str );    

        slider_value_set( &screen->time_slider, 0.0f );
        redraw = true;
        }    

    if( screen->cover_hover && input->scroll < 0 )
        {
        screen->big_pic = true;
        screen->big_redraw = true;
        redraw = true;
        screen->zoom_time = 0.0f;
        screen->big_x = 100;
        screen->big_y = 120;
        screen->big_w = 600;
        screen->big_h = 600;
        }

    if( screen->big_pic )
        {
        if( input->scroll > 0 ) 
            { 
            screen->big_pic = false;
            screen->big_redraw = false;
            redraw = true;
            }

        if( screen->pics_count > 0 ) 
            {
            if( input->mouse_x > screen->width / 2 + screen->width / 10 )
                {
                if( input->click )
                    {
                    if( screen->selected_pic == -1 ) screen->selected_pic = 1; else screen->selected_pic++;
                    if( screen->selected_pic >= screen->pics_count ) screen->selected_pic = 0;
                    redraw = true;
                    }
                }
            if( input->right )                 
                {
                if( screen->selected_pic == -1 ) screen->selected_pic = 1; else screen->selected_pic++;
                if( screen->selected_pic >= screen->pics_count ) screen->selected_pic = 0;
                redraw = true;
                }



            if( input->mouse_x < screen->width / 2 - screen->width / 10 )
                {
                if( input->click)
                    {
                    if( screen->selected_pic == -1 ) screen->pics_count = 1; else screen->selected_pic--;
                    if( screen->selected_pic < 0 ) screen->selected_pic = screen->pics_count - 1;
                    redraw = true;
                    }
                }
            if( input->left )
                {
                if( screen->selected_pic == -1 ) screen->pics_count = 1; else screen->selected_pic--;
                if( screen->selected_pic < 0 ) screen->selected_pic = screen->pics_count - 1;
                redraw = true;
                }
            }

        int width = 0;
        int height = 0;
        if( screen->cover && screen->selected_pic < 0 ) 
            {
            width = screen->cover->tex.width;
            height = screen->cover->tex.height;
            }
        if( screen->selected_pic >= 0 ) 
            {
            width = screen->pics[ screen->selected_pic ].bmp->tex.width;
            height = screen->pics[ screen->selected_pic ].bmp->tex.height;
            }

        float outw = screen->width - 20.0f;
        float aspect = (float) height / (float) width;        
        float outh = outw * aspect;

        if( outh > screen->height - 80.0f )
            {
            outh = screen->height - 80.0f;
            outw = outh / aspect;
            }
                
        float target_x1 = ( screen->width - outw ) / 2.0f;
        float target_y1 = 70;
        float target_x2 = target_x1 + outw;
        float target_y2 = target_y1 + outh;

        screen->big_x = (int) target_x1;
        screen->big_y = (int) target_y1;
        screen->big_w = (int) target_x2 - screen->big_x;
        screen->big_h = (int) target_y2 - screen->big_y;

        return redraw;
        }

    if( input->scroll || input->pgup || input->pgdown || input->up || input->down || input->home || input->end )
        {
        screen->scroll += input->scroll * 100;
        if( input->up ) screen->scroll -= 50;
        if( input->down ) screen->scroll += 50;
        if( input->pgup ) screen->scroll -= (int)screen->render->logical_height - 140;
        if( input->pgdown ) screen->scroll += (int)screen->render->logical_height - 140;
        if( input->home ) screen->scroll = 0;
        if( input->end ) screen->scroll = screen->layout_max_scroll;
        if( screen->scroll < 0 ) screen->scroll = 0;
        if( screen->scroll >= screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;
        redraw = true;
        }

    bool hover_artist = false;
    if( screen->text_artist )
        {
        if( input->mouse_x >= 750 && input->mouse_x <= 750 + screen->text_artist->width &&
            input->mouse_y >= 150 - screen->text_artist->baseline && input->mouse_y <= 150 - screen->text_artist->baseline + screen->text_artist->height )
            {
            hover_artist = true;
            }
        }
    if( screen->hover_artist != hover_artist )
        {
        screen->hover_artist = hover_artist;
        redraw = true;
        }

    if( input->click && hover_artist )
        {
        navigation->screen = SCREEN_ALBUMS;
        navigation->artist_id = screen->album_info.artist_id;
        }

    int prev_hover = screen->hover_track;
    screen->hover_track = -1;  
    int num_width = 0;
    int max_width = 0;
    for( int i = 0; i < screen->tracks_count; ++i )
        {
        track_t* track = &screen->tracks[ i ];     
        if( track->text_title && track->text_title->width > max_width ) max_width = track->text_title->width;
        if( track->text_tracknum && track->text_tracknum->width > num_width ) num_width = track->text_tracknum->width;
        if( track->text_track_artist && track->text_track_artist->width > max_width ) max_width = track->text_track_artist->width;
        }


    int hover_disc = -1;
    bool hover_discs = false;
    if( screen->discs_count > 1 )
        {
        for( int i = 0; i < screen->discs_count; ++i )
            {
            if( input->mouse_x >= 795 + i * 37 && input->mouse_x <= 795 + i * 37 + 50 &&
                input->mouse_y >= 180 && input->mouse_y <= 180+ 50 )
            hover_disc = i;
            }

        hover_discs = input->mouse_x >= 750 && input->mouse_x <= 800 &&
            input->mouse_y >= 180 && input->mouse_y <= 180+ 50;
        }
    redraw |= screen->hover_disc != hover_disc;
    screen->hover_disc = hover_disc;
           
    redraw |= screen->hover_discs != hover_discs;
    screen->hover_discs = hover_discs;
    
    if( screen->hover_disc >= 0 && input->click  )
        {
        redraw |= screen->selected_disc != screen->hover_disc;
        screen->selected_disc = screen->hover_disc;
        }
    
    if( screen->hover_discs && input->click  )
        {
        redraw |= screen->selected_disc != -1;
        screen->selected_disc = -1;
        }

    int ypos = screen->discs_count > 1 ? 25 : 0;
    for( int i = 0; i < screen->tracks_count; ++i )
        {
        track_t* track = &screen->tracks[ i ];
        if( screen->selected_disc >= 0 && screen->selected_disc < screen->discs_count && 
            screen->song_info[ i ].disc_number != screen->discs[ screen->selected_disc ] )
                continue;
        if( track->text_title )
            {
            bool various = false;
            if( screen->various && track->text_track_artist && strcmp( track->text_track_artist->string, screen->text_artist->string ) != 0 )
                various = true;
            int vspace = various ? 45 : 28;
            int x1 = 750 - 10;
            int x2 = 750 + num_width + 10 + max_width + 10;
            int y1 = 220 + 7 + ypos - vspace + ( various ? 18 : 0 );
            int y2 = y1 + vspace;
            if( track->text_tracklen ) x2 += 15 + track->text_tracklen->width;
            if( input->mouse_x >= x1 && input->mouse_x <= x2 && 
                input->mouse_y > 60 && input->mouse_y >= y1 && input->mouse_y <= y2 ) 
                {   
                screen->hover_track = i;
                screen->hover_track_width = x2 - x1;
                screen->hover_track_y = y1;
                screen->hover_track_h = y2 - y1;
                }
            ypos += vspace;
            }
        }
    if( screen->hover_track != prev_hover ) redraw |= true;

    if( screen->hover_track >= 0 && input->click )
        {
        if( screen->play_control->album_id != screen->album_id ) play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
        play_control_track( screen->play_control, screen->hover_track );
        }


    bool prev_cover_hover = screen->cover_hover;
    if( screen->cover ) 
        screen->cover_hover = input->mouse_x >= 100 && input->mouse_y >= 120 && input->mouse_x <= 700 && input->mouse_y <= 720;
    else 
        screen->cover_hover = false;
    redraw |= screen->cover_hover != prev_cover_hover;
    
    if( input->click && screen->cover_hover )
        {
        if( screen->play_control->album_id != screen->album_id )
            {
            play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
            }
        else
            {
            if( screen->play_control->is_paused )
                play_control_play( screen->play_control );
            else
                play_control_pause( screen->play_control );
            }
        }

    if( input->space )
        {
        if( screen->play_control->album_id == MUSICDB_INVALID_ID )
            {
            play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
            }
        }

    bool hover = false;

    redraw |= button_update( &screen->ctl_prev, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_prev, input->click_start_x, input->click_start_y, input->click) ) 
        {
        if( screen->play_control->album_id != screen->album_id ) play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
        play_control_prev( screen->play_control );
        }

    redraw |= button_update( &screen->ctl_rewind, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_rewind, input->click_start_x, input->click_start_y, input->lbutton ) ) 
        {
        if( screen->play_control->album_id != screen->album_id ) play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
        play_control_rewind( screen->play_control, true );
        screen->is_rewind = true;
        }
    if( screen->is_rewind && !input->lbutton ) 
        {
        if( screen->play_control->album_id != screen->album_id ) play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
        play_control_rewind( screen->play_control, false );
        screen->is_rewind = false;
        }

    redraw |= button_update( &screen->ctl_pause, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_pause, input->click_start_x, input->click_start_y, input->click) ) 
        {
        if( screen->play_control->album_id != screen->album_id ) 
            {
            play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
            }
        else
            {
            if( screen->play_control->is_paused )
                play_control_play( screen->play_control );
            else
                play_control_pause( screen->play_control );
            }
        redraw = true;
        }

    redraw |= button_update( &screen->ctl_stop, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_stop, input->click_start_x, input->click_start_y, input->click) ) 
        {
        play_control_stop( screen->play_control );
        }

    redraw |= button_update( &screen->ctl_ffwd, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_ffwd, input->click_start_x, input->click_start_y, input->lbutton ) ) 
        {
        if( screen->play_control->album_id != screen->album_id ) play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
        play_control_ffwd( screen->play_control, true );
        screen->is_ffwd = true;
        }
    if( screen->is_ffwd && !input->lbutton ) 
        {
        if( screen->play_control->album_id != screen->album_id ) play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
        play_control_ffwd( screen->play_control, false );
        screen->is_ffwd = false;
        }
    
    redraw |= button_update( &screen->ctl_next, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_next, input->click_start_x, input->click_start_y, input->click) ) 
        {
        play_control_next( screen->play_control );
        if( screen->play_control->album_id != screen->album_id ) play_control_album( screen->play_control, screen->album_id, screen->song_info, screen->tracks_count );
        }

    redraw |= button_update( &screen->ctl_loop, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_loop, input->click_start_x, input->click_start_y, input->click) ) 
        {
        play_control_loop( screen->play_control, !screen->play_control->loop );
        redraw = true;
        }

    redraw |= button_update( &screen->ctl_shuffle, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_shuffle, input->click_start_x, input->click_start_y, input->click) ) 
        {
        play_control_shuffle( screen->play_control, !screen->play_control->shuffle );
        redraw = true;
        }

    redraw |= button_update( &screen->ctl_repeat, input->mouse_x, input->mouse_y, &hover );
	if( hover && button_clicked( &screen->ctl_repeat, input->click_start_x, input->click_start_y, input->click) ) 
        {
        play_control_repeat( screen->play_control, !screen->play_control->repeat );
        redraw = true;
        }

    if( screen->play_control->album_id == screen->album_id && !screen->play_control->is_paused )
        {
        redraw |= screen->ctl_pause.bmp != screen->bmp_pause;
        screen->ctl_pause.bmp = screen->bmp_pause;
        }
    else
        {
        redraw |= screen->ctl_pause.bmp != screen->bmp_play;
        screen->ctl_pause.bmp = screen->bmp_play;
        }


    int pic_hover = -1;
    int x = 0;
    int y = 825;
    for( int i = 0; i < screen->pics_count; ++i )
        {
        if( input->mouse_x >= 95 + x && input->mouse_x <= 245 + x && input->mouse_y >= y - 4 && input->mouse_y <= y + 145 )
            {
            pic_hover = i;
            screen->pic_hover_x = 100 + x;
            screen->pic_hover_y = y;
            break;
            }
        x += 153;
        if( x + 140 > 600 )
            {
            x = 0;
            y += 150;
            }
        }
    if( screen->pic_hover != pic_hover )
        {
        screen->pic_hover = pic_hover;
        redraw = true;
        }

    if( pic_hover != -1 && input->click ) 
        {
        screen->selected_pic = pic_hover;
        redraw = true;
        }

    if( screen->play_control->album_id == screen->album_id ) 
        {
        float position = play_control_position( screen->play_control );
        float length = play_control_length( screen->play_control );
        if( length == 0.0f || position < 0.5f ) 
            { 
            length = (float)screen->song_info[ play_control_track_index( screen->play_control ) ].length_in_seconds;        
            }
        else 
            {
            musicdb_song_t* song_info = &screen->song_info[ play_control_track_index( screen->play_control ) ];
            int length_in_seconds = (int)( length + 0.5f );
            if( length_in_seconds != song_info->length_in_seconds )
                {
                musicdb_update_song_length( screen->db, song_info->id, length_in_seconds );
                song_info->length_in_seconds = length_in_seconds;
                int index = play_control_track_index( screen->play_control );
                if( index > 0 && index < screen->tracks_count )
                    {
                    track_t* track = &screen->tracks[ index ];
                    if( track->text_tracklen ) 
                        {
                        text_destroy( screen->render, track->text_tracklen );
                        int minutes = song_info->length_in_seconds / 60;
                        int seconds = song_info->length_in_seconds - 60 * minutes;
                        char str[ 16 ];
                        sprintf( str, "%d:%02d", minutes, seconds );
                        track->text_tracklen = text_create( screen->render, screen->font_length, str );    
                        }
                    }
                }
            }

        bool value_changed = false;
        redraw |= slider_update( &screen->time_slider, input->mouse_x, input->mouse_y, input->lbutton, &value_changed );
        if( value_changed )
            {
            position = length * slider_value( &screen->time_slider );
            play_control_position_set( screen->play_control, position );
            }

        int pos = (int)( position * 5.0f );
        if( ( pos != screen->prev_pos || position == length ) && !value_changed && !slider_is_dragging( &screen->time_slider ) ) 
            {
            slider_value_set( &screen->time_slider, position / length );
            }
        if( slider_is_dragging( &screen->time_slider ) )
            {
            position = length * slider_value( &screen->time_slider );
            }
        
        if( slider_is_dragging( &screen->time_slider ) || ( pos != screen->prev_pos || position == length ) || !screen->text_elapsed || !screen->text_remaining ) 
            {
            redraw |= true;
            screen->prev_pos = pos;
            int elapsed = (int) position;
            int remaining = ( (int)( length  ) ) - elapsed;
            char str[ 16 ];
            sprintf( str, "%d:%02d", elapsed / 60, elapsed % 60 );
            if( screen->text_elapsed ) text_destroy( screen->render, screen->text_elapsed );
            screen->text_elapsed = text_create( screen->render, screen->font_slider, str );    
        
            sprintf( str, "%d:%02d", remaining / 60, remaining % 60 );
            if( screen->text_remaining ) text_destroy( screen->render, screen->text_remaining );
            screen->text_remaining = text_create( screen->render, screen->font_slider, str );    

            }
        }
    else if( screen->prev_pos != 0 )
        {
        redraw |= true;
        screen->prev_pos = 0;
        slider_value_set( &screen->time_slider, 0.0f );
        if( screen->text_elapsed ) text_destroy( screen->render, screen->text_elapsed );
        screen->text_elapsed = text_create( screen->render, screen->font_slider, "0:00" );           
        if( screen->text_remaining ) text_destroy( screen->render, screen->text_remaining );
        screen->text_remaining = text_create( screen->render, screen->font_slider, "0:00" );    
        }

    if( play_control_track_index( screen->play_control ) != prev_track_index ) redraw = true;

    if( screen->pics_extract_track < screen->tracks_count ) redraw = true;

    return redraw;
    }


bool screen_tracks_draw( screen_tracks_t* screen )
    {
    bool redraw = false;
    int max_scroll = 0;
    render_scissor( screen->render, 0, 80, (int)screen->render->logical_width, (int)screen->render->logical_height );
    render_yoffset( screen->render, (float)screen->scroll );

    if( screen->album_id != MUSICDB_INVALID_ID && screen->album_info.id == screen->album_id )
        {
        char str[ 256 ];
        if( !screen->text_artist ) 
            {
            strcpy( str, screen->album_info.artist );
            while( strlen( str ) > 5 && text_width( screen->render, screen->font_artist, str ) > (int)screen->render->logical_width - 750 - 20 )
                {
                char* ptr = str + strlen( str ) - 1;
                while( ptr > str && !isalnum( (uint8_t) *ptr ) )
                    {
                    --ptr;
                    }
                *ptr = '\0';
                strcat( str, "..." );
                }
            screen->text_artist = text_create( screen->render, screen->font_artist, str );
            }
        if( !screen->text_album ) 
            {
            strcpy( str, screen->album_info.title );
            while( strlen( str ) > 5 && text_width( screen->render, screen->font_album, str ) > (int)screen->render->logical_width - 750 - 70 )
                {
                char* ptr = str + strlen( str ) - 1;
                while( ptr > str && !isalnum( (uint8_t) *ptr ) )
                    {
                    --ptr;
                    }
                *ptr = '\0';
                strcat( str, "..." );
                }
            screen->text_album = text_create( screen->render, screen->font_album, str );
            }
        if( !screen->text_year && screen->album_info.year > 0 ) 
            {
            sprintf( str, "%d", screen->album_info.year );
            screen->text_year = text_create( screen->render, screen->font_year, str );
            }
        }


    if( screen->cover_delay > 0)
        {
        --screen->cover_delay;
        redraw = true;
        if( screen->cover_delay == 0 )
            {
            if( screen->album_thumb.id != MUSICDB_INVALID_ID && file_exists( screen->album_thumb.source_filename ) )
                {
                redraw = true;
                size_t mp3size = file_size( screen->album_thumb.source_filename );
                mmap_t* mmap= mmap_open_read_only( screen->album_thumb.source_filename, mp3size );
                if( mmap )
                    {
                    size_t size = id3tag_size( mmap_data( mmap ) );
                    if( size > 0 ) 
                        { 
                        id3tag_t* tag = id3tag_load( mmap_data( mmap ), size, ID3TAG_FIELD_PICS, 0 );
                        if( tag && tag->pics_count > 0 ) 
                            {               
                            void const* img_data = tag->pics[ screen->album_thumb.source_image_index >= 0 ? screen->album_thumb.source_image_index : 0 ].data;
                            size_t img_size = tag->pics[ screen->album_thumb.source_image_index >= 0 ? screen->album_thumb.source_image_index : 0 ].size;
                            screen->cover_hash = calc_hash( (void*)img_data, img_size );
                            int w, h, c;
                            stbi_uc* img = stbi_load_from_memory( (stbi_uc const*) img_data, (int) img_size, &w, &h, &c, 4 );
                            if( img ) 
                                {
                                if( screen->cover ) bitmap_destroy( screen->render, screen->cover );

                                if( w != h ) 
                                    {
                                    int d = w > h ? w : h;
                                    stbi_uc* new_img = (stbi_uc*) malloc( d * d * 4 );
                                    memset( new_img, 0, d * d * 4 );
        
                                    float ratio = w > h ? ( (float)h ) / ( (float) d ) : ( (float)w ) / ( (float) d );

                                    if( ratio > 0.9f )
                                        {
                                        stbir_resize_uint8( (unsigned char*) img, w, h, 0, (unsigned char*) new_img, d, d, 0, 4 );
                                        }
                                    else {
                                        int mx = ( d - w ) / 2;
                                        int my = ( d - h ) / 2;
        
                                        for( int y = 0; y < h; ++y )
                                            {
                                            memcpy( new_img + 4 * mx + 4 * d * ( y + my ), img + 4 * w * y, 4 * w );
                                            }
                                        }

                                    stbi_image_free( img );
                                    img = new_img;
                                    w = d;
                                    h = d;
                                    }

                                screen->cover = bitmap_create_raw( screen->render, (uint32_t*) img, w, h );
                                free( img );
                                }
                            id3tag_free( tag );
                            }
                        }
                    mmap_close( mmap ); 
                    }
                }
            screen->album_thumb.id = MUSICDB_INVALID_ID;
            }
        }
    else
        {
        if( screen->pics_extract_track < screen->tracks_count )
            {
            if( !screen->pics_extract_tag )
                {
                size_t mp3size = file_size( screen->song_info[ screen->pics_extract_track ].filename );
                mmap_t* mmap= mmap_open_read_only( screen->song_info[ screen->pics_extract_track ].filename, mp3size );
                if( mmap )
                    {
                    size_t size = id3tag_size( mmap_data( mmap ) );
                    if( size > 0 ) 
                        { 
                        screen->pics_extract_tag = id3tag_load( mmap_data( mmap ), size, ID3TAG_FIELD_PICS, 0 );
                        }
                    mmap_close( mmap ); 
                    }
                }
            if( !screen->pics_extract_tag ) 
                {
                ++screen->pics_extract_track;
                screen->pics_extract_index = 0;
                }
            else if( screen->pics_extract_index >= screen->pics_extract_tag->pics_count )
                {
                id3tag_free( screen->pics_extract_tag );
                screen->pics_extract_tag = 0;
                ++screen->pics_extract_track;
                screen->pics_extract_index = 0;
                }
            else
                {
            
                void const* img_data = screen->pics_extract_tag->pics[ screen->pics_extract_index ].data;
                size_t img_size = screen->pics_extract_tag->pics[ screen->pics_extract_index ].size;
                uint64_t hash = calc_hash( (void*) img_data, img_size );
                if( hash != screen->cover_hash )
                    {
                    bool already_exist = false;
                    for( int i = 0; i < screen->pics_count; ++i )
                        {
                        if( hash == screen->pics[ i ].hash )
                            {
                            already_exist = true;
                            break;
                            }
                        }
                    if( !already_exist )
                        {
                        if( screen->pics_count == 0 )
                            {
                            int index = screen->pics_count++;
                            screen->pics[ index ].hash = screen->cover_hash;
                            screen->pics[ index ].pic_type =  ID3TAG_PIC_TYPE_COVER_FRONT;
                            screen->pics[ index ].bmp = screen->cover;
                            }
                        int w, h, c;
                        stbi_uc* img = stbi_load_from_memory( (stbi_uc const*) img_data, (int) img_size, &w, &h, &c, 4 );
                        if( img ) 
                            {
                            int index = screen->pics_count++;
                            screen->pics[ index ].hash = hash;
                            screen->pics[ index ].pic_type =  screen->pics_extract_tag->pics[ screen->pics_extract_index ].pic_type;
                            screen->pics[ index ].bmp = bitmap_create_raw( screen->render, (uint32_t*) img, w, h );
                            free( img );

                            int const pic_type_sort_order[] = { 4, 5, 6, 0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 13, 14, 
                                15, 16, 255, 17, 18, 19, };
                            uint32_t const sort_order_count = sizeof( pic_type_sort_order ) / sizeof( *pic_type_sort_order );

                            int new_pos = index;
                            for( int i = 0; i < screen->pics_count; ++i )
                                {
                                int a = (uint32_t) screen->pics[ i ].pic_type < sort_order_count ? pic_type_sort_order[ screen->pics[ i ].pic_type ] : screen->pics[ i ].pic_type;
                                int b = (uint32_t) screen->pics[ index ].pic_type < sort_order_count ? pic_type_sort_order[ screen->pics[ index ].pic_type ] : screen->pics[ index ].pic_type;
                                if( a > b )
                                    {
                                    new_pos = i;
                                    break;
                                    }
                                }
                            if( new_pos != index )
                                {
                                pic_t tmp = screen->pics[ index ];
                                memmove( &screen->pics[ new_pos + 1 ], &screen->pics[ new_pos ], sizeof( tmp ) * ( screen->pics_count - new_pos - 1) );
                                screen->pics[ new_pos ] = tmp;
                                }
                            }
                        }
                    }
                ++screen->pics_extract_index;
                }
            }
        }

    int x = 0;
    int y = 825;
    for( int i = 0; i < screen->pics_count; ++i )
        {
        if( screen->pics[ i ].bmp ) bitmap_draw( screen->render, screen->pics[ i ].bmp, 100 + x, y, 140, 140, 0xffffffff );
        x += 153;
        if( x + 140 > 600 )
            {
            x = 0;
            y += 150;
            }
        }
    if( x != 0 ) y += 150;
    if( y > max_scroll ) max_scroll = y;

    if( screen->pic_hover != -1 && screen->pic_hover != screen->selected_pic )
        {
        bitmap_draw( screen->render, screen->frame, screen->pic_hover_x - 8, screen->pic_hover_y - 8, 156, 156, 0xffd98b1a );
        }

    if( screen->text_artist ) text_draw( screen->render, screen->text_artist, 750, 150, screen->hover_artist ? 0xffd98b1a : 0xffffffff );
    if( screen->text_album ) text_draw( screen->render, screen->text_album, 750, 180, 0xffd98b1a );
    if( screen->text_album && screen->text_year ) text_draw( screen->render, screen->text_year, 750 + 15 + screen->text_album->width, 180, 0xff736d64 );

    if( screen->hover_track >= 0 )
        {
        render_draw_rect( screen->render, 750 - 10, screen->hover_track_y, screen->hover_track_width, screen->hover_track_h, 0x40736d64 );
        }

    int num_width = 0;
    for( int i = 0; i < screen->tracks_count; ++i )
        {
        track_t* track = &screen->tracks[ i ];
        
        if( !track->text_tracknum ) 
            {
            char str[ 16 ];
            sprintf( str, "%d", screen->song_info[ i ].track_number );
            track->text_tracknum = text_create( screen->render, screen->font_title, str );    
            }
        if( track->text_tracknum->width > num_width ) num_width = track->text_tracknum->width;

        if( !track->text_title ) 
            {
            char str[ 256 ];
            sprintf( str, "%s", screen->song_info[ i ].title );
            while( strlen( str ) > 5 && text_width( screen->render, screen->font_title, str ) > (int)screen->render->logical_width - 750 - 90 )
                {
                char* ptr = str + strlen( str ) - 1;
                while( ptr > str && !isalnum( (uint8_t) *ptr ) )
                    {
                    --ptr;
                    }
                *ptr = '\0';
                strcat( str, "..." );
                }
            track->text_title = text_create( screen->render, screen->font_title, str );    
            }
        
        if( !track->text_tracklen ) 
            {
            int minutes = screen->song_info[ i ].length_in_seconds / 60;
            int seconds = screen->song_info[ i ].length_in_seconds - 60 * minutes;
            char str[ 16 ];
            sprintf( str, "%d:%02d", minutes, seconds );
            track->text_tracklen = text_create( screen->render, screen->font_length, str );    
            }

        if( !track->text_track_artist && screen->various ) 
            {
            char const* artist_name = screen->song_info[ i ].artist;
            if( strnicmp( artist_name, screen->album_info.artist, strlen( screen->album_info.artist ) ) == 0 ) 
                {
                artist_name += strlen( screen->album_info.artist );
                while( *artist_name && !isalnum( (uint8_t)*artist_name ) ) ++artist_name;
                }
            if( *artist_name ) 
                {
                char str[ 256 ];
                strcpy( str, artist_name );
                while( strlen( str ) > 5 && text_width( screen->render, screen->font_track_artist, str ) > (int)screen->render->logical_width - 750 - 40 )
                    {
                    char* ptr = str + strlen( str ) - 1;
                    while( ptr > str && !isalnum( (uint8_t) *ptr ) )
                        {
                        --ptr;
                        }
                    *ptr = '\0';
                    strcat( str, "..." );
                    }
                track->text_track_artist = text_create( screen->render, screen->font_track_artist, str );    
                }
            }
        }
   
    if( screen->discs_count > 1 )
        {
        bitmap_draw( screen->render, screen->all_discs, 750, 180, 50, 50, screen->selected_disc < 0 ? 0xffffffff : screen->hover_discs ? 0xffd98b1a : 0xff736d64);
        for( int i = 0; i < screen->discs_count; ++i )
            {
            if( screen->disc_num[ i ] == 0 )
                {
                char str[ 16 ];
                sprintf( str, "%d", screen->discs[ i ] );
                screen->disc_num[ i ] = text_create( screen->render, screen->font_title, str );    
                }
            
            text_draw( screen->render, screen->disc_num[ i ], 795 + i * 37 + 25 - (int)( screen->disc_num[ i ]->w_acc / 2.0f ), 185 + 25, screen->selected_disc == i ? 0xffffffff : screen->hover_disc == i ? 0xffd98b1a : 0xff736d64 );
            bitmap_draw( screen->render, screen->circle, 795 + i * 37, 180, 50, 50, screen->selected_disc == i ? 0xffffffff : screen->hover_disc == i ? 0xffd98b1a : 0xff736d64 );
            }
        }

    int ypos = screen->discs_count > 1 ? 25 : 0;
    for( int i = 0; i < screen->tracks_count; ++i )
        {
        track_t* track = &screen->tracks[ i ];
        uint32_t col = ( screen->play_control->album_id == screen->album_id && i == play_control_track_index( screen->play_control ) ) ? 0xffd98b1a : 0xffffffff;

        if( screen->selected_disc >= 0 && screen->selected_disc < screen->discs_count && 
            screen->song_info[ i ].disc_number != screen->discs[ screen->selected_disc ] )
                continue;

        if( track->text_title )
            {
            
            bool various = false;
            if( screen->various && track->text_track_artist && strcmp( track->text_track_artist->string, screen->text_artist->string ) != 0 )
                various = true;
            int vspace = various ? 45 : 28;

            if( track->text_tracknum ) 
                text_draw( screen->render, track->text_tracknum, 750 + ( num_width - track->text_tracknum->width ) / 2, 220 + ypos, col );

            text_draw( screen->render, track->text_title, 750 + num_width + 10, 220 + ypos, col );

            if( track->text_tracklen )
                text_draw( screen->render, track->text_tracklen, 750 + num_width + 10 + track->text_title->width + 15, 220 + ypos - 1, 0xff736d64 );

            if( track->text_track_artist && various )
                text_draw( screen->render, track->text_track_artist, 750 + num_width + 10, 220 + ypos + 20, 0xff736d64 );

            ypos += vspace;
            }
        }
    if( 220 + ypos > max_scroll ) max_scroll = 220 + ypos;
        

    if( screen->album_id != MUSICDB_INVALID_ID )
        {
        button_draw( &screen->ctl_prev, text_col, text_col_highlighted );   
        button_draw( &screen->ctl_rewind, screen->is_rewind ? text_col_highlighted : text_col, text_col_highlighted );
        button_draw( &screen->ctl_pause, screen->cover_hover ? text_col_highlighted : text_col, text_col_highlighted );
        button_draw( &screen->ctl_stop, text_col, text_col_highlighted );
        button_draw( &screen->ctl_ffwd, screen->is_ffwd ? text_col_highlighted : text_col, text_col_highlighted );
        button_draw( &screen->ctl_next, text_col, text_col_highlighted );

        button_draw( &screen->ctl_loop, screen->play_control->loop ? text_col_highlighted : text_col, screen->play_control->loop ? 0xffffffff : text_col_highlighted );
        button_draw( &screen->ctl_shuffle, screen->play_control->shuffle ? text_col_highlighted : text_col, screen->play_control->shuffle ? 0xffffffff : text_col_highlighted );
        button_draw( &screen->ctl_repeat, screen->play_control->repeat ? text_col_highlighted : text_col, screen->play_control->repeat ? 0xffffffff : text_col_highlighted );

        int elapsed_width = (int)strlen( screen->text_elapsed->string ) * 8;
        int elapsed_pos = screen->time_slider.x - elapsed_width - 12;
        int remaining_pos = screen->time_slider.x + screen->time_slider.w + 12;
        text_draw( screen->render, screen->text_elapsed, elapsed_pos, 794, 0xff736d64 );
        text_draw( screen->render, screen->text_remaining, remaining_pos, 794, 0xff736d64 );
        slider_draw( &screen->time_slider, 0xff736d64, text_col_highlighted );
        }

    if( screen->big_pic )
        {
        render_yoffset( screen->render, 0.0f );
        render_scissor_off( screen->render );
        render_scissor( screen->render, 0, 58, (int)screen->render->logical_width, (int)screen->render->logical_height );
        if( screen->cover || screen->selected_pic >= 0 ) render_draw_rect( screen->render, 0, 0, screen->width, screen->height, 0xc0000000 );
        if( screen->cover && screen->selected_pic < 0 ) bitmap_draw_raw( screen->render, screen->cover, (float)screen->big_x, (float)screen->big_y, (float)screen->big_w, (float)screen->big_h, 0xffffffff );
        if( screen->selected_pic >= 0 && screen->pics[ screen->selected_pic ].bmp ) bitmap_draw_raw( screen->render, screen->pics[ screen->selected_pic ].bmp, (float)screen->big_x, (float)screen->big_y, (float)screen->big_w, (float)screen->big_h, 0xffffffff );
        if( screen->big_redraw ) 
            {
            redraw = true;
            screen->big_redraw = false;
            }
        }
    else
        {
        if( screen->cover && screen->selected_pic < 0 ) bitmap_draw( screen->render, screen->cover, 100, 120, 600, 600, 0xffffffff );
        if( screen->selected_pic >= 0 && screen->pics[ screen->selected_pic ].bmp ) bitmap_draw( screen->render, screen->pics[ screen->selected_pic ].bmp, 100, 120, 600, 600, 0xffffffff );
        }

    render_scissor_off( screen->render );
    render_yoffset( screen->render, 0.0f );
    max_scroll -= (int)screen->render->logical_height;
    max_scroll += 40;
    if( max_scroll < 0 ) max_scroll = 0;
    screen->layout_max_scroll = max_scroll;
    return redraw;
    }
