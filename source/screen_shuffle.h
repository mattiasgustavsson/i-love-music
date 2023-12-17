


typedef struct screen_shuffle_item_t
    {
    text_t* title_text;
    char title_string[ 256 ];
        
    text_t* artist_text;
    char artist_string[ 256 ];
        
    text_t* album_text;
    char album_string[ 256 ];

    uint32_t album_id;

    bitmap_t* thumbnail;
} screen_shuffle_item_t;


typedef struct screen_shuffle_t
    {
    app_t* app;
    render_t* render;
    font_t* font_title;
    font_t* font_artist;
    font_t* font_album;
    int width;
    int height;
    int scroll;
    int highlighted;

    int layout_size;
    int layout_hmargin;
    int layout_hspacing;
    int layout_vspacing;
    int layout_hcount;
    int layout_vcount;
    int layout_offset;
    int layout_max_scroll;

    bool force_resize;

    button_t btn_play;
    button_t btn_pause;
    button_t btn_next;
    button_t btn_reshuffle;
    button_t btn_tracks;
    button_t btn_albums;
    button_t btn_remove;
    button_t btn_add_all;
    button_t btn_remove_all;

    int track_index;

    int items_count;
    screen_shuffle_item_t* items;
    } screen_shuffle_t;


bool screen_shuffle_init( screen_shuffle_t* screen, app_t* app, render_t* render )
    {
    memset( screen, 0, sizeof( *screen ) );
    screen->app = app;
    screen->render = render;

    if( !( screen->font_title = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 30 ) ) ) return false;
    if( !( screen->font_artist = font_create( screen->render, "gui/fonts/AlegreyaSans-Bold.ttf", 26 ) ) ) return false;
    if( !( screen->font_album = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 24 ) ) ) return false;

    screen->width = (int)render->logical_width;
    screen->height = (int)render->logical_height;
    screen->scroll = 0;
    screen->highlighted = -1;


    int x = 0;
    int y = 0;

    y += 100;
    button_init( &screen->btn_play, render, load_app_bitmap( render, "gui/icons/play.png", ".cache/gui/icons/play.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_play, x, y, 100, 100);
    button_init( &screen->btn_pause, render, load_app_bitmap( render, "gui/icons/pause.png", ".cache/gui/icons/pause.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_pause, x, y, 100, 100);

    y += 100;
    button_init( &screen->btn_next, render, load_app_bitmap( render, "gui/icons/next.png", ".cache/gui/icons/next.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_next, x, y, 100, 100);

    y += 100;
    button_init( &screen->btn_reshuffle, render, load_app_bitmap( render, "gui/icons/refresh.png", ".cache/gui/icons/refresh.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_reshuffle, x, y, 100, 100);

    y += 100;
    button_init( &screen->btn_tracks, render, load_app_bitmap( render, "gui/icons/list.png", ".cache/gui/icons/list.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_tracks, x, y, 100, 100);

    y += 100;
    button_init( &screen->btn_albums, render, load_app_bitmap( render, "gui/icons/discs.png", ".cache/gui/icons/discs.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_albums, x, y, 100, 100);

    button_init( &screen->btn_remove, render, load_app_bitmap( render, "gui/icons/remove.png", ".cache/gui/icons/remove.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_remove, screen->width - 110 - 10, 105 + 88 * 2, 70, 70);

    button_init( &screen->btn_add_all, render, load_app_bitmap( render, "gui/icons/discs.png", ".cache/gui/icons/discs.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_add_all, x, screen->height - 220, 100, 100);

    button_init( &screen->btn_remove_all, render, load_app_bitmap( render, "gui/icons/close.png", ".cache/gui/icons/close.mip", global_max_w, global_max_h ) );
    button_size( &screen->btn_remove_all, x, screen->height - 120, 100, 100);

    screen->items_count = 0;

    screen->layout_size = 250;
    screen->layout_hmargin = 25;
    screen->layout_hspacing = 25;
    screen->layout_vspacing = 90;
    screen->layout_hcount = (int)( ( screen->render->logical_width - screen->layout_hmargin * 2 ) / ( screen->layout_size + screen->layout_hspacing * 2 ) );
    if( screen->layout_hcount < 1 ) screen->layout_hcount = 1;
    screen->layout_vcount = (int)( ( screen->render->logical_height - 100 ) / ( screen->layout_size + screen->layout_vspacing ) );
    screen->layout_offset = (int)( ( screen->render->logical_width - screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) ) / 2 );
    screen->layout_max_scroll = 3 * ( screen->items_count / screen->layout_hcount - ( screen->layout_vcount - 1 ) ) - 1;
    if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;

    //screen_shuffle_item_t* all_artists = &screen->items[ screen->items_count++ ];
    //all_artists->genre_id = MUSICDB_INVALID_ID;
    //strcpy( all_artists->genre_string, "All artists" );
//    all_artists->genre_text = text_create( screen->render, screen->font_genre, "All artists");   
//    *all_artists->album_count_string = '\0';
//    all_artists->album_count_text = 0;
//    all_artists->thumb_id = MUSICDB_INVALID_ID;
    
    return true;
    }


void screen_shuffle_term( screen_shuffle_t* screen )
    {
	button_term( &screen->btn_next );
	button_term( &screen->btn_pause );
	button_term( &screen->btn_play );
	button_term( &screen->btn_remove );
	button_term( &screen->btn_add_all );
	button_term( &screen->btn_remove_all );
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_shuffle_item_t* item = &screen->items[ i ];
        if( item->thumbnail ) bitmap_destroy( screen->render, item->thumbnail );
        if( item->title_text ) text_destroy( screen->render, item->title_text );
        if( item->album_text ) text_destroy( screen->render, item->album_text );
        if( item->artist_text ) text_destroy( screen->render, item->artist_text );
        }
    //font_destroy( screen->render, screen->font_albums );
    //font_destroy( screen->render, screen->font_genre );
    free( screen->items );
    }



/*
bool screen_shuffle_update_thumbs( screen_shuffle_t* screen, music_db_t* musicdb )
    {
    bool result = false;
    int pic_count = 0;
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_shuffle_item_t* item = &screen->items[ i ];
        if( !item->bitmap && item->thumb_id != MUSICDB_INVALID_ID )
            {
            int mip_width, mip_height, w, h;
            bitmap_t* thumb = musicdb_get_thumbnail( musicdb, item->thumb_id, &w, &h, &mip_width, &mip_height, screen->render );
            if( thumb )
                {
                item->bitmap = thumb;
                if( !item->bitmap ) 
                    item->thumb_id = MUSICDB_INVALID_ID;
                else
                    result = true;
                }
            pic_count++;
            }
        }

    return result;
    }
*/

void screen_shuffle_refresh( screen_shuffle_t* screen, music_db_t* musicdb )
    {
    int song_count = 0;
    musicdb_song_t* songs = musicdb_shuffle_get( musicdb, &song_count, 0, -1 );

    if( screen->items ) 
        {
        for( int i = 0; i < screen->items_count; ++i ) 
            {
            screen_shuffle_item_t* item = &screen->items[ i ];
            text_destroy( screen->render, item->title_text );
            text_destroy( screen->render, item->artist_text );
            text_destroy( screen->render, item->album_text );
            if( item->thumbnail ) 
                {
                bitmap_destroy( screen->render, item->thumbnail );
                }
            }
        free( screen->items );
        }

    screen->items_count = song_count;
    screen->items = (screen_shuffle_item_t*) malloc( sizeof( screen_shuffle_item_t ) * song_count );
   
    char str[ 256 ];
    for( int i = 0; i < song_count; ++i )
        {
        musicdb_song_t* song = &songs[ i ];
        screen_shuffle_item_t* item = &screen->items[ i ];

        item->title_text = NULL;
        strncpy( str, song->title, sizeof( str ) - 4 );
        str[ sizeof( str ) - 4 ] = '\0';        
        strcpy( item->title_string, str );
        

        item->artist_text = NULL;
        strncpy( str, song->artist, sizeof( str ) - 4 );
        str[ sizeof( str ) - 4 ] = '\0';        
        strcpy( item->artist_string, str );

        item->album_text = NULL;
        strncpy( str, song->album, sizeof( str ) - 4 );
        str[ sizeof( str ) - 4 ] = '\0';        
        strcpy( item->album_string, str );

        item->album_id = song->album_id;

        item->thumbnail = NULL;
        }
    
    
    musicdb_shuffle_release( musicdb, songs );

    /*
    screen->items_count = song_count;

    screen_albums_item_t* new_items = (screen_shuffle_item_t*) malloc( sizeof( *new_items ) * song_count );

    int new_items_count = 0;

    char str[ 256 ];
    for( int i = 0; i < song_count; ++i )
        {
        musicdb_shuffle_song_t* shuffle = &shuffle_songs[ i ];
        screen_albums_item_t* item = &new_items[ new_items_count++ ];

        item->title_text = 0;
        strncpy( str, shuffle->title, sizeof( str ) - 4 );
        str[ sizeof( str ) - 4 ] = '\0';        
        strcpy( item->title_string, str );
        
        item->artist_text = 0;
        strncpy( str, shuffle->artist, sizeof( str ) - 4 );
        str[ sizeof( str ) - 4 ] = '\0';   
        strcpy( item->artist_string, str );

       
        item->track_count_text = 0;
        if( album->compilation )
            sprintf( str, "Compilation  (%d songs)", album->track_count );
        else if( album->year )
            sprintf( str, "%d  (%d songs)", album->year, album->track_count );
        else
            sprintf( str, "(%d songs)", album->track_count );
        strcpy( item->track_count_string, str );
            
        item->bitmap = 0;
        }


    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_albums_item_t* item = &screen->items[ i ];
        if( item->bitmap ) bitmap_destroy( screen->render, item->bitmap );
        if( item->title_text )text_destroy( screen->render, item->title_text );
        if( item->artist_text )text_destroy( screen->render, item->artist_text );
        if( item->track_count_text )text_destroy( screen->render, item->track_count_text );
        }

    free( screen->items );
    screen->items = new_items;
    screen->items_count = new_items_count;

    musicdb_shuffle_release( musicdb, songs );
*/

    /*
    screen_shuffle_item_t* new_items = (screen_shuffle_item_t*) malloc( sizeof( *new_items ) * ( genres_count + 1 ) );
    int new_items_count = 0;

    char str[ 256 ];
    new_items[ new_items_count++ ] = screen->items[ 0 ];
    memset( &screen->items[ 0 ], 0, sizeof( screen_shuffle_item_t ) );
    screen->items[ 0 ].genre_id = MUSICDB_INVALID_ID;

    screen_shuffle_item_t* all_artists = &new_items[ 0 ];
    sprintf( str, "%d albums", total_album_count );
    if( strcmp( str, all_artists->album_count_string ) != 0 )
        {
        if( all_artists->album_count_text ) text_destroy( screen->render, all_artists->album_count_text );
        all_artists->album_count_text = text_create( screen->render, screen->font_albums, str );
        strcpy( all_artists->album_count_string, str );
        }
    
    for( int i = 0; i < genres_count; ++i )
        {
        musicdb_genre_t* genre = &genres[ i ];
        char const* label = remap_genre ? genre->remapped_label: genre->label;
        if( strlen( label ) > 200 ) continue;
        bool found = false;
        for( int j = 0; j < screen->items_count; ++j )
            {
            if( strcmp( screen->items[ j ].genre_string, label ) == 0 )
                {
                screen_shuffle_item_t* item = &new_items[ new_items_count++ ];
                *item = screen->items[ j ];
                memset( &screen->items[ j ], 0, sizeof( screen_shuffle_item_t ) );
                screen->items[ j ].genre_id = MUSICDB_INVALID_ID;

                sprintf( str, "%d albums", genre->album_count );               
                if( strcmp( item->album_count_string, str ) != 0 )
                    {
                    if( item->album_count_text ) text_destroy( screen->render, item->album_count_text );
                    item->album_count_text = 0;
                    strcpy( item->album_count_string, str );
                    }
                found = true;
                break;
                }
            }
        if( !found )
            {
            screen_shuffle_item_t* item = &new_items[ new_items_count++ ];
            item->genre_id = genre->id;

            strcpy( item->genre_string, label );
            item->genre_text = 0;

            sprintf( str, "%d albums", genre->album_count );
            strcpy( item->album_count_string, str );
            item->album_count_text = 0;

            item->thumb_id = remap_genre ? genre->remapped_thumb_id : genre->thumbnail_id;
            item->bitmap = 0;
            }
        }

    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_shuffle_item_t* item = &screen->items[ i ];
        if( item->genre_text ) text_destroy( screen->render, item->genre_text );
        if( item->album_count_text ) text_destroy( screen->render, item->album_count_text );
        }
    
    free( screen->items );
    screen->items = new_items;
    screen->items_count = new_items_count;

    screen->layout_max_scroll = 3 * ( screen->items_count / screen->layout_hcount - ( screen->layout_vcount - 1 ) ) - 1;
    if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;
    
    scroll_index = 0;
    if( *scroll_item )
        {
        for( int i = 0; i < screen->items_count; ++i )
            {
            screen_shuffle_item_t* item = &screen->items[ i ];
            if( stricmp( item->genre_string, scroll_item ) == 0 )
                scroll_index = i;
            }
        }
    screen->scroll = scroll_offset + 3 * ( scroll_index / screen->layout_hcount );

    musicdb_genres_release( musicdb, genres );
    screen->force_resize = true;
    screen_shuffle_update_thumbs( screen, musicdb );
    */
    }


typedef struct screen_shuffle_navigation_t
    {
    screen_t screen;
    uint32_t selected_album_id;
    } screen_shuffle_navigation_t;


bool screen_shuffle_update( screen_shuffle_t* screen, screen_shuffle_navigation_t* navigation, bool resize, input_t const* input, music_db_t* musicdb, play_control_t* play_control )
    {
    bool redraw = false;
    navigation->screen = SCREEN_SHUFFLE;
    navigation->selected_album_id = MUSICDB_INVALID_ID;
    
    if( play_control->album_id == MUSICDB_INVALID_ID && play_control->track_index != screen->track_index && play_control->tracks_count) {
        screen->track_index = play_control->track_index;        
        redraw = true;
    }

    if( resize || screen->force_resize || screen->width != (int)screen->render->logical_width || screen->height != (int)screen->render->logical_height )
        {
        screen->force_resize = false;
        screen->width = (int)screen->render->logical_width;
        screen->height = (int)screen->render->logical_height;
        screen->highlighted = -1;

        int x = 0;
        int y = 0;

        y += 80;
        button_size( &screen->btn_play, x, y, 100, 100 );
        button_modify_bounds( &screen->btn_play, 0, 10, 0, -10 );
        button_size( &screen->btn_pause, x, y, 100, 100 );
        button_modify_bounds( &screen->btn_pause, 0, 10, 0, -10 );

        y += 80;
        button_size( &screen->btn_next, x, y, 100, 100 );
        button_modify_bounds( &screen->btn_next, 0, 10, 0, -10 );

        y += 80;
        button_size( &screen->btn_reshuffle, x + 25, y + 25, 50, 50 );
        button_modify_bounds( &screen->btn_reshuffle, -25, -15, 25, 15 );

        y += 80;
        button_size( &screen->btn_tracks, x + 25, y + 25, 50, 50 );
        button_modify_bounds( &screen->btn_tracks, -25, -15, 25, 15 );

        y += 80;
        button_size( &screen->btn_albums, x + 25, y + 25, 50, 50 );
        button_modify_bounds( &screen->btn_albums, -25, -15, 25, 15 );

        button_size( &screen->btn_remove, screen->width - 110 - 10, 105 + 88 * 2, 70, 70 );
        button_modify_bounds( &screen->btn_remove, -25, -15, 25, 15 );

        button_size( &screen->btn_add_all, x + 26, screen->height - 200, 70, 70 );
        button_modify_bounds( &screen->btn_add_all, -25, -15, 25, 15 );

        button_size( &screen->btn_remove_all, x + 26, screen->height - 100, 70, 70 );
        button_modify_bounds( &screen->btn_remove_all, -25, -15, 25, 15 );

        int scroll_item = ( screen->scroll / 3 ) * screen->layout_hcount;
        int scroll_offset = screen->scroll % 3;

        screen->layout_hcount = (int)( ( screen->render->logical_width - screen->layout_hmargin * 2 ) / ( screen->layout_size + screen->layout_hspacing * 2 ) );
        if( screen->layout_hcount < 1 ) screen->layout_hcount = 1;
        screen->layout_vcount = (int)( ( screen->render->logical_height - 100 ) / ( screen->layout_size + screen->layout_vspacing ) );
        screen->layout_offset = (int)( ( screen->render->logical_width - screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) ) / 2 );

        int row_count = ( screen->items_count + ( screen->layout_hcount - 1 ) ) / screen->layout_hcount;
        int scroll_end = row_count * ( screen->layout_size + screen->layout_vspacing );
        int screen_steps = ( (int)screen->render->logical_height - 100 ) / ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
        int scroll_steps = ( 3 * scroll_end ) / ( screen->layout_size + screen->layout_vspacing );
        screen->layout_max_scroll = scroll_steps - screen_steps;
        if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;
        screen->scroll = scroll_offset + 3 * ( scroll_item / screen->layout_hcount );
        if( screen->scroll > screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;

        for( int i = 0; i < screen->items_count; ++i )
            {
            screen_shuffle_item_t* item = &screen->items[ i ];
            if( item->title_text ) text_destroy( screen->render, item->title_text );
            item->title_text = 0;
            if( item->album_text ) text_destroy( screen->render, item->album_text );
            item->album_text = 0;
            if( item->artist_text ) text_destroy( screen->render, item->artist_text );
            item->artist_text = 0;
            }

        redraw = true;
        }

    if( input->scroll || input->pgup || input->pgdown || input->up || input->down || input->home || input->end )
        {
        screen->scroll += input->scroll;
        if( input->up ) screen->scroll--;
        if( input->down ) screen->scroll++;
        if( input->pgup ) screen->scroll -= ( screen->layout_vcount ) * 3 - 1;
        if( input->pgdown ) screen->scroll += ( screen->layout_vcount ) * 3 - 1;
        if( input->home ) screen->scroll = 0;
        if( input->end ) screen->scroll = screen->layout_max_scroll;
        if( screen->scroll < 0 ) screen->scroll = 0;
        if( screen->scroll >= screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;
        redraw = true;
        }

    int highlighted = -1;
/*    int x = 0;
    int y = -screen->scroll * ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
/*    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_shuffle_item_t* item = &screen->items[ i ];
         if( input->mouse_x >= screen->layout_hspacing + x + screen->layout_offset - 8 && 
             input->mouse_x < screen->layout_hspacing + x + screen->layout_offset + screen->layout_size + 8 && 
             input->mouse_y > 60 && input->mouse_y >= y + 100 - 8 && input->mouse_y < y + 100 + screen->layout_size + 60 + 8 )
            {
            highlighted = i;
            if( input->click )
                {
                 if( input->click_start_x >= screen->layout_hspacing + x + screen->layout_offset - 8 && 
                     input->click_start_x < screen->layout_hspacing + x + screen->layout_offset + screen->layout_size + 8 && 
                     input->click_start_y > 60 && input->click_start_y >= y + 100 - 8 && input->click_start_y < y + 100 + screen->layout_size + 60 + 8 ) 
                    {
                    navigation->screen = SCREEN_ARTISTS;
                    navigation->selected_genre_id = item->genre_id;
                    navigation->genre_thumb_id = item->thumb_id;
                    }
                }
            break;
            }
        x += screen->layout_size + screen->layout_hspacing * 2;
        if( x >= screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) )
            {
            x = 0;
            y += screen->layout_size + screen->layout_vspacing;
            }
        }
*/
    if( highlighted != screen->highlighted )
        {
        screen->highlighted = highlighted;
        redraw = true;
        }


    bool hover = false;

    redraw |= button_update( &screen->btn_play, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click && ( play_control->album_id != MUSICDB_INVALID_ID || play_control->tracks_count == 0 ) )
        {
        int tracks_count = 0;
        musicdb_song_t* tracks = musicdb_shuffle_get( musicdb, &tracks_count, 0, -1 );
        if( tracks && tracks_count ) 
            {
            play_control_pause( play_control );
            play_control_shuffle_list( play_control, tracks, tracks_count );
            play_control_pause( play_control );
            play_control_track( play_control, screen->track_index );
            musicdb_shuffle_release( musicdb, tracks );
            }
        redraw = true;
        }
    else if( hover && input->click && play_control->is_paused && play_control->album_id == MUSICDB_INVALID_ID && play_control->tracks_count > 0 ) 
        {
        play_control_play( play_control );
        }
    else 
        {
        redraw |= button_update( &screen->btn_pause, input->mouse_x, input->mouse_y, &hover );
	    if( hover && input->click && !play_control->is_paused && play_control->album_id == MUSICDB_INVALID_ID && play_control->tracks_count > 0 )
            {
            play_control_pause( play_control );
            redraw = true;
            }
    }

    redraw |= button_update( &screen->btn_next, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click ) 
        {
        if( play_control->album_id == MUSICDB_INVALID_ID && play_control->tracks_count ) 
            {
            play_control_next( play_control );
            }
        else 
            {
            int tracks_count = 0;
            musicdb_song_t* tracks = musicdb_shuffle_get( musicdb, &tracks_count, 0, -1 );
            if( tracks && tracks_count ) 
                {
                play_control_pause( play_control );
                play_control_shuffle_list( play_control, tracks, tracks_count );
                play_control_pause( play_control );
                play_control_track( play_control, screen->track_index );
                musicdb_shuffle_release( musicdb, tracks );
                }
            }
        redraw = true;
        }

    redraw |= button_update( &screen->btn_reshuffle, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click ) 
        {
        musicdb_shuffle_reshuffle( musicdb );
        screen_shuffle_refresh( screen, musicdb );
        int tracks_count = 0;
        musicdb_song_t* tracks = musicdb_shuffle_get( musicdb, &tracks_count, 0, -1 );
        if( tracks && tracks_count ) 
            {
            screen->track_index = 0;
            play_control_pause( play_control );
            play_control_shuffle_list( play_control, tracks, tracks_count );
            musicdb_shuffle_release( musicdb, tracks );
            }
        redraw = true;
        }

    /*
    redraw |= button_update( &screen->btn_tracks, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click ) 
        {
        redraw = true;
        }
    */

    /*
    redraw |= button_update( &screen->btn_albums, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click ) 
        {
        redraw = true;
        }
    */

    
    redraw |= button_update( &screen->btn_remove, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click && screen->items_count ) 
        {
        redraw = true;
        musicdb_shuffle_remove( musicdb, screen->track_index );
        screen_shuffle_refresh( screen, musicdb );
        int tracks_count = 0;
        musicdb_song_t* tracks = musicdb_shuffle_get( musicdb, &tracks_count, 0, -1 );
        if( tracks && tracks_count ) 
            {
            bool resume = !( play_control->is_paused || play_control->tracks_count == 0 );
            play_control->play_thread->ignore_resume = !resume;
            play_control_pause( play_control );
            play_control_shuffle_list( play_control, tracks, tracks_count );
            play_control_pause( play_control );
            play_control->track_index = -1;
            play_control_track( play_control, screen->track_index );
            if( !resume ) 
                {
                play_control_pause( play_control );
                }
            musicdb_shuffle_release( musicdb, tracks );
            }
        redraw = true;
        }

    redraw |= button_update( &screen->btn_add_all, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click ) {
        musicdb_shuffle_add_all( musicdb );
        musicdb_shuffle_reshuffle( musicdb );
        screen->track_index = 0;
        int tracks_count = 0;
        musicdb_song_t* tracks = musicdb_shuffle_get( musicdb, &tracks_count, 0, -1 );
        if( tracks && tracks_count ) 
            {
            play_control_pause( play_control );
            play_control_shuffle_list( play_control, tracks, tracks_count );
            play_control_pause( play_control );
            play_control_track( play_control, screen->track_index );
            musicdb_shuffle_release( musicdb, tracks );
            }
        screen_shuffle_refresh( screen, musicdb );
        redraw = true;

    }

    redraw |= button_update( &screen->btn_remove_all, input->mouse_x, input->mouse_y, &hover );
	if( hover && input->click ) {
        musicdb_shuffle_remove_all( musicdb );
        musicdb_shuffle_reshuffle( musicdb );
        screen->track_index = 0;
        play_control_stop( play_control );
        screen_shuffle_refresh( screen, musicdb );
        redraw = true;

    }

    return redraw;
    }


bool screen_shuffle_draw( screen_shuffle_t* screen, music_db_t* musicdb, play_control_t* play_control )
    {
    render_scissor( screen->render, 0, 80, (int)screen->render->logical_width, (int)screen->render->logical_height );

    render_draw_rect( screen->render, 110, 100 + 88 * 2 - 6, screen->width - 110 - 20, 80 + 12, 0x40736d64 );

    if( screen->items_count ) {
        int y = 100;
        int start_index = screen->track_index - 2;
        for( int i = 0; y < screen->height ; ++i ) {
            int index = ( i + start_index + screen->items_count ) % screen->items_count;
            screen_shuffle_item_t* item = &screen->items[ index ];

            uint32_t alpha = i >= 2 ? 0xff000000 : 0x80000000;

            if( !item->thumbnail ) 
                {
                int w, h, mip_width, mip_height;
                bitmap_t* thumb = musicdb_get_album_thumbnail( musicdb, item->album_id, &w, &h, &mip_width, &mip_height, screen->render );
                if( thumb )
                    {
                    item->thumbnail = thumb;
                    }
                }
            if( item->thumbnail ) 
                {
                bitmap_draw( screen->render, item->thumbnail, 140, y, 80, 80, alpha | 0xffffff );
                }

            if( !item->title_text ) item->title_text = text_create( screen->render, screen->font_title, item->title_string );
            text_draw( screen->render, item->title_text, 240, y + 23, alpha | 0xd98b1a );

            if( !item->artist_text ) item->artist_text = text_create( screen->render, screen->font_artist, item->artist_string );
            text_draw( screen->render, item->artist_text, 240, y + 48, alpha |  0xffffff );

            if( !item->album_text ) item->album_text = text_create( screen->render, screen->font_album, item->album_string );
            text_draw( screen->render, item->album_text, 240, y + 73, alpha | 0x736d64 );

            y += 88;
        }
    }

    if( !play_control->is_paused && play_control->album_id == MUSICDB_INVALID_ID && play_control->tracks_count > 0 ) 
        button_draw( &screen->btn_pause, text_col, text_col_highlighted );
    else
        button_draw( &screen->btn_play, text_col, text_col_highlighted );

    button_draw( &screen->btn_next, text_col, text_col_highlighted );
    button_draw( &screen->btn_reshuffle, text_col, text_col_highlighted );
    //button_draw( &screen->btn_tracks, text_col, text_col_highlighted );
    //button_draw( &screen->btn_albums, text_col, text_col_highlighted );
    
    button_draw( &screen->btn_add_all, text_col, text_col_highlighted );
    button_draw( &screen->btn_remove_all, text_col, text_col_highlighted );


    if( screen->items_count ) 
        {
        button_draw( &screen->btn_remove, text_col, text_col_highlighted );
        }

    render_scissor_off( screen->render );
    return false;
    }
