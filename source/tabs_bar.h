typedef struct tabs_bar_t
    {
    render_t* render;
    play_control_t* play_control;
    font_t* font;

    bool showing_current_album;
    int active_tab_index;
    int highlighted_tab_index;

    text_t* genres_text;
    int genres_width;

    text_t* artists_text;
    int artists_width;

    text_t* albums_text;
    int albums_width;

    text_t* tracks_text;
    int tracks_width;

    text_t* shuffle_text;
    int shuffle_width;

    bitmap_t* bmp_notab_shadow;
    bitmap_t* bmp_notab_highlight;

    bitmap_t* bmp_leftmost_active_shadow;
    bitmap_t* bmp_leftmost_active_highlight;
    bitmap_t* bmp_leftmost_nonactive_shadow;
    bitmap_t* bmp_leftmost_nonactive_highlight;
    
    bitmap_t* bmp_middle_active_shadow;
    bitmap_t* bmp_middle_active_highlight;
    bitmap_t* bmp_middle_nonactive_shadow;
    bitmap_t* bmp_middle_nonactive_highlight;

    bitmap_t* bmp_rightmost_active_shadow;
    bitmap_t* bmp_rightmost_active_highlight;
    bitmap_t* bmp_rightmost_nonactive_shadow;
    bitmap_t* bmp_rightmost_nonactive_highlight;

    bitmap_t* bmp_active_to_nonactive_shadow;
    bitmap_t* bmp_active_to_nonactive_highlight;
    bitmap_t* bmp_nonactive_to_active_shadow;
    bitmap_t* bmp_nonactive_to_active_highlight;
    bitmap_t* bmp_nonactive_to_nonactive_shadow;
    bitmap_t* bmp_nonactive_to_nonactive_highlight;

//    edit_t* search_box;

    button_t speaker;
    slider_t volume;

    button_t power;

    button_t ctl_prev;
    button_t ctl_pause;
    button_t ctl_next;
    button_t ctl_playing;
    button_t ctl_shuffle_add;
    button_t ctl_shuffle_remove;

    bitmap_t* bmp_play;
    bitmap_t* bmp_speaker_off;
    bool is_paused;
    } tabs_bar_t;



tabs_bar_t* tabs_bar_create( render_t* render, play_control_t* play_control, slider_bitmaps_t slider_bitmaps )
    {
    tabs_bar_t* bar = (tabs_bar_t*) malloc( sizeof( tabs_bar_t ) );
    memset( bar, 0, sizeof( *bar ) );
    bar->render = render;
    bar->play_control = play_control;

    bar->active_tab_index = 1;
    bar->highlighted_tab_index = 0;

    if( !( bar->font = font_create( bar->render, "gui/fonts/AlegreyaSans-Medium.ttf", 30 ) ) ) { free( bar ); return 0; }

    int const size = global_max_w;

    bar->bmp_leftmost_active_shadow = load_app_bitmap( bar->render, "gui/tabs/leftmost_active_shadow.png", ".cache/gui/tabs/leftmost_active_shadow.mip", size, size );
    bar->bmp_leftmost_active_highlight = load_app_bitmap( bar->render, "gui/tabs/leftmost_active_highlight.png", ".cache/gui/tabs/leftmost_active_highlight.mip", size, size );

    bar->bmp_leftmost_nonactive_shadow = load_app_bitmap( bar->render, "gui/tabs/leftmost_nonactive_shadow.png", ".cache/gui/tabs/leftmost_nonactive_shadow.mip", size, size  );
    bar->bmp_leftmost_nonactive_highlight = load_app_bitmap( bar->render, "gui/tabs/leftmost_nonactive_highlight.png", ".cache/gui/tabs/leftmost_nonactive_highlight.mip", size, size );

    bar->bmp_middle_active_shadow = load_app_bitmap( bar->render, "gui/tabs/middle_active_shadow.png", ".cache/gui/tabs/middle_active_shadow.mip", size, size );
    bar->bmp_middle_active_highlight = load_app_bitmap( bar->render, "gui/tabs/middle_active_highlight.png", ".cache/gui/tabs/middle_active_highlight.mip", size, size );

    bar->bmp_middle_nonactive_shadow = load_app_bitmap( bar->render, "gui/tabs/middle_nonactive_shadow.png", ".cache/gui/tabs/middle_nonactive_shadow.mip", size, size );
    bar->bmp_middle_nonactive_highlight = load_app_bitmap( bar->render, "gui/tabs/middle_nonactive_highlight.png", ".cache/gui/tabs/middle_nonactive_highlight.mip", size, size );

    bar->bmp_rightmost_active_shadow = load_app_bitmap( bar->render, "gui/tabs/rightmost_active_shadow.png", ".cache/gui/tabs/rightmost_active_shadow.mip", size, size );
    bar->bmp_rightmost_active_highlight = load_app_bitmap( bar->render, "gui/tabs/rightmost_active_highlight.png", ".cache/gui/tabs/rightmost_active_highlight.mip", size, size );

    bar->bmp_rightmost_nonactive_shadow = load_app_bitmap( bar->render, "gui/tabs/rightmost_nonactive_shadow.png", ".cache/gui/tabs/rightmost_nonactive_shadow.mip", size, size );
    bar->bmp_rightmost_nonactive_highlight = load_app_bitmap( bar->render, "gui/tabs/rightmost_nonactive_highlight.png", ".cache/gui/tabs/rightmost_nonactive_highlight.mip", size, size );

    bar->bmp_active_to_nonactive_shadow = load_app_bitmap( bar->render, "gui/tabs/active_to_nonactive_shadow.png", ".cache/gui/tabs/active_to_nonactive_shadow.mip", size, size );
    bar->bmp_active_to_nonactive_highlight = load_app_bitmap( bar->render, "gui/tabs/active_to_nonactive_highlight.png", ".cache/gui/tabs/active_to_nonactive_highlight.mip", size, size );

    bar->bmp_nonactive_to_active_shadow = load_app_bitmap( bar->render, "gui/tabs/nonactive_to_active_shadow.png", ".cache/gui/tabs/nonactive_to_active_shadow.mip", size, size );
    bar->bmp_nonactive_to_active_highlight = load_app_bitmap( bar->render, "gui/tabs/nonactive_to_active_highlight.png", ".cache/gui/tabs/nonactive_to_active_highlight.mip", size, size );

    bar->bmp_nonactive_to_nonactive_shadow = load_app_bitmap( bar->render, "gui/tabs/nonactive_to_nonactive_shadow.png", ".cache/gui/tabs/nonactive_to_nonactive_shadow.mip", size, size );
    bar->bmp_nonactive_to_nonactive_highlight = load_app_bitmap( bar->render, "gui/tabs/nonactive_to_nonactive_highlight.png", ".cache/gui/tabs/nonactive_to_nonactive_highlight.mip", size, size );

    bar->bmp_notab_shadow = load_app_bitmap( bar->render, "gui/tabs/notab_shadow.png", ".cache/gui/tabs/notab_shadow.mip", size, size );
    bar->bmp_notab_highlight = load_app_bitmap( bar->render, "gui/tabs/notab_highlight.png", ".cache/gui/tabs/notab_highlight.mip", size, size );

//     bar->search_box = edit_create( render, bar->font );
//     bar->search_box->x = 550;
//     bar->search_box->y = 44; 

    button_init( &bar->power, render, load_app_bitmap( render, "gui/icons/power.png", ".cache/gui/icons/power.mip", global_max_w, global_max_h ) );
    button_size( &bar->power, (int)bar->render->logical_width - 52, 9, 44, 44 );
    button_override_bounds( &bar->power, (int) bar->render->logical_width - 60, 0, 60, 60 );


    int x = (int)bar->render->logical_width - 80;
    int y = 2;

    x -= 40;
    button_init( &bar->ctl_next, render, load_app_bitmap( render, "gui/icons/next.png", ".cache/gui/icons/next.mip", global_max_w, global_max_h ) );
    button_size( &bar->ctl_next, x, y, 64, 64 );
    button_modify_bounds( &bar->ctl_next, 12, -2, -12, -2 );

    x -= 40;
    button_init( &bar->ctl_pause, render, load_app_bitmap( render, "gui/icons/pause.png", ".cache/gui/icons/pause.mip", global_max_w, global_max_h ) );
    button_size( &bar->ctl_pause, x, y, 64, 64 );
    button_modify_bounds( &bar->ctl_pause, 12, -2, -12, -2 );

    x -= 40;
    button_init( &bar->ctl_prev, render, load_app_bitmap( render, "gui/icons/prev.png", ".cache/gui/icons/prev.mip", global_max_w, global_max_h ) );
    button_size( &bar->ctl_prev, x, y, 64, 64 );
    button_modify_bounds( &bar->ctl_prev, 12, -2, -12, -2 );

    x -= 40;
    button_init( &bar->ctl_playing, render, load_app_bitmap( render, "gui/icons/down.png", ".cache/gui/icons/down.mip", global_max_w, global_max_h ) );
    button_size( &bar->ctl_playing, x, y + 4, 56, 56 );
    button_modify_bounds( &bar->ctl_playing, 6, -6, 6, 0 );

    x -= 120;
    slider_init( &bar->volume, render, slider_bitmaps, x, y, 120, 20 );
    slider_value_set( &bar->volume, 1.0f );
    
    x -= 40;
    button_init( &bar->speaker, render, load_app_bitmap( render, "gui/icons/speaker.png", ".cache/gui/icons/speaker.mip", global_max_w, global_max_h ) );
    button_size( &bar->speaker, x, y + 4, 56, 56 );
    button_modify_bounds( &bar->speaker, 6, -6, 6, 0 );

    x -= 55;
    button_init( &bar->ctl_shuffle_add, render, load_app_bitmap( render, "gui/icons/add.png", ".cache/gui/icons/add.mip", global_max_w, global_max_h ) );
    button_size( &bar->ctl_shuffle_add, x, y + 4, 56, 56 );
    button_modify_bounds( &bar->ctl_shuffle_add, 6, -6, 6, 0 );

    button_init( &bar->ctl_shuffle_remove, render, load_app_bitmap( render, "gui/icons/remove.png", ".cache/gui/icons/remove.mip", global_max_w, global_max_h ) );
    button_size( &bar->ctl_shuffle_remove, x, y + 4, 56, 56 );
    button_modify_bounds( &bar->ctl_shuffle_remove, 6, -6, 6, 0 );

    bar->bmp_play = load_app_bitmap( render, "gui/icons/play.png", ".cache/gui/icons/play.mip", global_max_w, global_max_h ) ;
    bar->bmp_speaker_off = load_app_bitmap( render, "gui/icons/speaker_off.png", ".cache/gui/icons/speaker_off.mip", global_max_w, global_max_h ) ;

	return bar;
    }


void tabs_bar_destroy( tabs_bar_t* bar )
    {
    bitmap_destroy( bar->render, bar->bmp_speaker_off );
    bitmap_destroy( bar->render, bar->bmp_play );
    button_term( &bar->ctl_shuffle_remove );
    button_term( &bar->ctl_shuffle_add );
	button_term( &bar->ctl_playing );
	button_term( &bar->ctl_prev );
	button_term( &bar->ctl_pause );
	button_term( &bar->ctl_next );
	button_term( &bar->power );
	button_term( &bar->speaker );
    slider_term( &bar->volume );

//     if( bar->search_box ) edit_destroy( bar->render, bar->search_box );
    if( bar->bmp_leftmost_active_shadow ) bitmap_destroy( bar->render, bar->bmp_leftmost_active_shadow );
    if( bar->bmp_leftmost_active_highlight ) bitmap_destroy( bar->render, bar->bmp_leftmost_active_highlight );
    if( bar->bmp_leftmost_nonactive_shadow ) bitmap_destroy( bar->render, bar->bmp_leftmost_nonactive_shadow );
    if( bar->bmp_leftmost_nonactive_highlight ) bitmap_destroy( bar->render, bar->bmp_leftmost_nonactive_highlight );
    if( bar->bmp_middle_active_shadow ) bitmap_destroy( bar->render, bar->bmp_middle_active_shadow );
    if( bar->bmp_middle_active_highlight ) bitmap_destroy( bar->render, bar->bmp_middle_active_highlight );
    if( bar->bmp_middle_nonactive_shadow ) bitmap_destroy( bar->render, bar->bmp_middle_nonactive_shadow );
    if( bar->bmp_middle_nonactive_highlight ) bitmap_destroy( bar->render, bar->bmp_middle_nonactive_highlight );
    if( bar->bmp_rightmost_active_shadow ) bitmap_destroy( bar->render, bar->bmp_rightmost_active_shadow );
    if( bar->bmp_rightmost_active_highlight ) bitmap_destroy( bar->render, bar->bmp_rightmost_active_highlight );
    if( bar->bmp_rightmost_nonactive_shadow ) bitmap_destroy( bar->render, bar->bmp_rightmost_nonactive_shadow );
    if( bar->bmp_rightmost_nonactive_highlight ) bitmap_destroy( bar->render, bar->bmp_rightmost_nonactive_highlight );
    if( bar->bmp_active_to_nonactive_shadow ) bitmap_destroy( bar->render, bar->bmp_active_to_nonactive_shadow );
    if( bar->bmp_active_to_nonactive_highlight ) bitmap_destroy( bar->render, bar->bmp_active_to_nonactive_highlight );
    if( bar->bmp_nonactive_to_active_shadow ) bitmap_destroy( bar->render, bar->bmp_nonactive_to_active_shadow );
    if( bar->bmp_nonactive_to_active_highlight ) bitmap_destroy( bar->render, bar->bmp_nonactive_to_active_highlight );
    if( bar->bmp_nonactive_to_nonactive_shadow ) bitmap_destroy( bar->render, bar->bmp_nonactive_to_nonactive_shadow );
    if( bar->bmp_nonactive_to_nonactive_highlight ) bitmap_destroy( bar->render, bar->bmp_nonactive_to_nonactive_highlight );
    if( bar->bmp_notab_shadow ) bitmap_destroy( bar->render, bar->bmp_notab_shadow );
    if( bar->bmp_notab_highlight ) bitmap_destroy( bar->render, bar->bmp_notab_highlight );
    if( bar->genres_text ) text_destroy( bar->render, bar->genres_text );
    if( bar->artists_text ) text_destroy( bar->render, bar->artists_text );
    if( bar->albums_text ) text_destroy( bar->render, bar->albums_text );
    if( bar->tracks_text ) text_destroy( bar->render, bar->tracks_text );
    if( bar->shuffle_text ) text_destroy( bar->render, bar->shuffle_text );
    if( bar->font ) font_destroy( bar->render, bar->font );
    free( bar );
    }


typedef struct tabs_bar_navigation_t
    {
    screen_t screen;
    uint32_t album_id;
    bool hide;
    } tabs_bar_navigation_t;


bool tabs_bar_update( tabs_bar_t* bar, tabs_bar_navigation_t* navigation, app_input_t appinput, input_t const* input, bool resize, uint32_t album_screen_album_id, uint32_t genre_screen_genre_id, bool supress_track_change, music_db_t* musicdb )
    {
    (void) appinput;
    navigation->album_id = MUSICDB_INVALID_ID;
    navigation->hide = false;
    bar->showing_current_album = album_screen_album_id != MUSICDB_INVALID_ID && album_screen_album_id == bar->play_control->album_id;

    bool redraw = false;
    if( resize )
        {
        int x = (int)bar->render->logical_width - 80;
        int y = 2;

        x -= 40;
        button_size( &bar->ctl_next, x, y, 64, 64 );
        button_modify_bounds( &bar->ctl_next, 12, -2, -12, -2 );

        x -= 40;
        button_size( &bar->ctl_pause, x, y, 64, 64 );
        button_modify_bounds( &bar->ctl_pause, 12, -2, -12, -2 );

        x -= 40;
        button_size( &bar->ctl_prev, x, y, 64, 64 );
        button_modify_bounds( &bar->ctl_prev, 12, -2, -12, -2 );

        x -= 40;
        button_size( &bar->ctl_playing, x, y, 64, 64 );
        button_modify_bounds( &bar->ctl_playing, 12, -2, -12, -2 );

        x -= 100;
        slider_size( &bar->volume, x, y + 21,  100, 20 );
    
        x -= 50;
        button_size( &bar->speaker, x, y + 4, 56, 56 );
        button_modify_bounds( &bar->speaker, 12, -2, -12, -2 );

        x -= 55;
        button_size( &bar->ctl_shuffle_add, x, y, 64, 64 );
        button_modify_bounds( &bar->ctl_shuffle_add, 12, -2, -12, -2 );

        button_size( &bar->ctl_shuffle_remove, x, y, 64, 64 );
        button_modify_bounds( &bar->ctl_shuffle_remove, 12, -2, -12, -2 );

        button_size( &bar->power, (int)bar->render->logical_width - 52, 9, 44, 44 );        
        button_override_bounds( &bar->power, (int) bar->render->logical_width - 60, 0, 60, 60 );
//         if( bar->search_box->text ) text_destroy( bar->render, bar->search_box->text );
        if( bar->genres_text ) text_destroy( bar->render, bar->genres_text );
        if( bar->artists_text ) text_destroy( bar->render, bar->artists_text );
        if( bar->albums_text ) text_destroy( bar->render, bar->albums_text );
        if( bar->tracks_text ) text_destroy( bar->render, bar->tracks_text );
        if( bar->shuffle_text ) text_destroy( bar->render, bar->shuffle_text );
//         bar->search_box->text = 0;
        bar->genres_text = 0;
        bar->artists_text = 0;
        bar->albums_text = 0;
        bar->tracks_text = 0;
        bar->shuffle_text = 0;
        redraw = true;
        }

//     redraw = edit_update( bar->search_box, appinput, ctrl, shift, mouse_x, mouse_y );

    int highlighted_tab_index = 0;
    if( input->mouse_y < 60 )
        {
        float x_next;

        float x = 45 + 10;
        x_next = x + bar->genres_width + 18;
        if( input->mouse_x > x && input->mouse_x < x_next ) highlighted_tab_index = 1;
        x_next += 4;

        x = x_next;
        x_next = x + bar->artists_width + 18;
        if( input->mouse_x > x && input->mouse_x < x_next ) highlighted_tab_index = 2;
        x_next += 4;

        x = x_next;
        x_next = x + bar->albums_width + 18;
        if( input->mouse_x > x && input->mouse_x < x_next ) highlighted_tab_index = 3;
        x_next += 4;

        x = x_next;
        x_next = x + bar->tracks_width + 18;
        if( input->mouse_x > x && input->mouse_x < x_next ) highlighted_tab_index = 4;
        x_next += 4;

        x = x_next;
        x_next = x + bar->shuffle_width + 18;
        if( input->mouse_x > x && input->mouse_x < x_next ) highlighted_tab_index = 5;
        x_next += 4;
        }

    if( highlighted_tab_index != bar->highlighted_tab_index )
        {
        bar->highlighted_tab_index = highlighted_tab_index;
        redraw = true;
        }

    if( input->click && highlighted_tab_index != 0 )
        {
        if( highlighted_tab_index == 1 ) navigation->screen = SCREEN_GENRES;
        if( highlighted_tab_index == 2 ) navigation->screen = SCREEN_ARTISTS;
        if( highlighted_tab_index == 3 ) navigation->screen = SCREEN_ALBUMS;
        if( highlighted_tab_index == 4 ) navigation->screen = SCREEN_TRACKS;
        if( highlighted_tab_index == 5 ) navigation->screen = SCREEN_SHUFFLE;
        }
	
    bool hover_power = false;
    redraw |= button_update( &bar->power, input->mouse_x, input->mouse_y, &hover_power );
	if( hover_power && input->click ) navigation->hide = true;

    

    bool hover = false;

    redraw |= button_update( &bar->ctl_playing, input->mouse_x, input->mouse_y, &hover );
	if( bar->play_control->album_id != MUSICDB_INVALID_ID && hover && input->click )
        {
        navigation->screen = SCREEN_TRACKS;
        navigation->album_id = bar->play_control->album_id;
        }

    redraw |= button_update( &bar->ctl_shuffle_add, input->mouse_x, input->mouse_y, &hover );
	if( bar->active_tab_index == 4 && hover && input->click && album_screen_album_id != MUSICDB_INVALID_ID )
        {
        navigation->screen = SCREEN_SHUFFLE;
        musicdb_shuffle_add_album( musicdb, album_screen_album_id );
        }
	if( bar->active_tab_index == 2 && hover && input->click && genre_screen_genre_id != MUSICDB_INVALID_ID )
        {
        navigation->screen = SCREEN_SHUFFLE;
        musicdb_shuffle_add_genre( musicdb, genre_screen_genre_id );
        }

    redraw |= button_update( &bar->ctl_shuffle_remove, input->mouse_x, input->mouse_y, &hover );
	if( bar->active_tab_index == 4 && hover && input->click )
        {
        navigation->screen = SCREEN_SHUFFLE;
        }

    redraw |= button_update( &bar->ctl_prev, input->mouse_x, input->mouse_y, &hover );
	if( ( hover && input->click ) || input->key_prev_track || ( !supress_track_change && input->left ) ) 
        { 
        play_control_prev( bar->play_control );
        redraw = true;
        }

    redraw |= button_update( &bar->ctl_pause, input->mouse_x, input->mouse_y, &hover );
	if( ( hover && input->click ) || input->key_play_pause || input->space ) 
        {
        if( bar->play_control->is_paused )
            play_control_play( bar->play_control );
        else
            play_control_pause( bar->play_control );
        }

    if( ( bar->is_paused && !bar->play_control->is_paused ) || ( !bar->is_paused && bar->play_control->is_paused ) )
        {
        bar->is_paused = bar->play_control->is_paused;
        bitmap_t* t = bar->ctl_pause.bmp;
        bar->ctl_pause.bmp = bar->bmp_play;
        bar->bmp_play = t;
        redraw = true;
        }

    redraw |= button_update( &bar->ctl_next, input->mouse_x, input->mouse_y, &hover );
	if( ( hover && input->click ) || input->key_next_track || ( !supress_track_change && input->right ) ) 
        {
        play_control_next( bar->play_control );
        redraw = true;
        }

    if( !g_mute ) 
    {
        bool changed = false;
        redraw |= slider_update( &bar->volume, input->mouse_x, input->mouse_y, input->lbutton, &changed );
        if( slider_is_dragging( &bar->volume ) ) 
            {
            g_global_volume = bar->volume.drag_value;
            }
        else
            {
            g_global_volume = slider_value( &bar->volume );
            }
    }

    bool hover_speaker = false;
    redraw |= button_update( &bar->speaker, input->mouse_x, input->mouse_y, &hover_speaker );
	if( hover_speaker && input->click ) 
        {
        redraw = true;
        g_mute = !g_mute;
        bitmap_t* t = bar->speaker.bmp;
        bar->speaker.bmp = bar->bmp_speaker_off;
        bar->bmp_speaker_off = t;
        if( g_mute ) 
            {
            slider_value_set( &bar->volume, 0.0f );
            }
        else
            {
            slider_value_set( &bar->volume, g_global_volume );
            }
        }

    return redraw;
    }


float tabs_internal_draw( tabs_bar_t* bar, float x, float y, float height, text_t* text, int text_width, bool is_highlighted, bool is_active, bool prev_active, bool next_active, bool is_leftmost, bool is_rightmost )
    {
    (void) is_active, prev_active, next_active, is_leftmost, is_rightmost;
    float init_x = x;

    // start
    if( is_leftmost )
        {
        if( is_active )
            x += bitmap_draw_tab( bar->render, bar->bmp_leftmost_active_shadow, bar->bmp_leftmost_active_highlight, x, y, height, 0xffffffff ) - 0.5f;    
        else
            x += bitmap_draw_tab( bar->render, bar->bmp_leftmost_nonactive_shadow, bar->bmp_leftmost_nonactive_highlight, x, y, height, 0xffffffff );    
        }

    // middle
    float text_x = is_leftmost ? x : x - 13;     
    if( is_active )
        x += bitmap_draw_tab_tiled( bar->render, bar->bmp_middle_active_shadow, bar->bmp_middle_active_highlight, x, y, height, (float) text_width - ( is_leftmost ? 10.0f : 25.0f ), 0xffffffff );
    else
        x += bitmap_draw_tab_tiled( bar->render, bar->bmp_middle_nonactive_shadow, bar->bmp_middle_nonactive_highlight, x, y, height, (float) text_width - ( is_leftmost ? 10.0f : 25.0f ), 0xffffffff );

    // end
    if( is_rightmost )
        {
        if( is_active )
            x += bitmap_draw_tab( bar->render, bar->bmp_rightmost_active_shadow, bar->bmp_rightmost_active_highlight, x, y, height, 0xffffffff );
        else
            x += bitmap_draw_tab( bar->render, bar->bmp_rightmost_nonactive_shadow, bar->bmp_rightmost_nonactive_highlight, x, y, height, 0xffffffff );
        }
    else
        {
        if( is_active && !next_active )
            x += bitmap_draw_tab( bar->render, bar->bmp_active_to_nonactive_shadow, bar->bmp_active_to_nonactive_highlight, x, y, height, 0xffffffff );
        else if( !is_active && next_active )
            x += bitmap_draw_tab( bar->render, bar->bmp_nonactive_to_active_shadow, bar->bmp_nonactive_to_active_highlight, x, y, height, 0xffffffff );
        else 
            x += bitmap_draw_tab( bar->render, bar->bmp_nonactive_to_nonactive_shadow, bar->bmp_nonactive_to_nonactive_highlight, x, y, height, 0xffffffff );
        }

    // text
    text_draw( bar->render, text, (int) text_x, (int) y + 34, is_active ? text_col_active : ( is_highlighted ? text_col_highlighted : text_col ) );

    return x - init_x;
    }


void tabs_bar_draw( tabs_bar_t* bar )
    {
    if( !bar->genres_text ) 
        {
        bar->genres_width = text_width( bar->render, bar->font, "Genres" );
        bar->genres_text = text_create( bar->render, bar->font, "Genres" );   
        }

    if( !bar->artists_text ) 
        {
        bar->artists_width = text_width( bar->render, bar->font, "Artists" );
        bar->artists_text = text_create( bar->render, bar->font, "Artists" );   
        }

    if( !bar->albums_text ) 
        {
        bar->albums_width = text_width( bar->render, bar->font, "Albums" );
        bar->albums_text = text_create( bar->render, bar->font, "Albums" );   
        }

    if( !bar->tracks_text ) 
        {
        bar->tracks_width = text_width( bar->render, bar->font, "Tracks" );
        bar->tracks_text = text_create( bar->render, bar->font, "Tracks" );   
        }

    if( !bar->shuffle_text ) 
        {
        bar->shuffle_width = text_width( bar->render, bar->font, "Shuffle" );
        bar->shuffle_text = text_create( bar->render, bar->font, "Shuffle" );   
        }

    float bar_height = 50.0f;
    float x = 0.0f;
    float y = 10;

    bool const leftmost = true;
    bool const rightmost = true;
    bool const not_leftmost = false;
    bool const not_rightmost = false;

    bool tabs_active[ 16 ] = { false };
    tabs_active[ bar->active_tab_index ] = true;
    
    uint32_t const bar_bg = 0xff101010;
    render_draw_rect( bar->render, 0, 0, (int)bar->render->logical_width, 11, bar_bg );

    // settings space
    x += bitmap_draw_tab_tiled( bar->render, bar->bmp_notab_shadow, bar->bmp_notab_highlight, x, y, bar_height, 45, 0xffffffff );

    int i = 1;

    // Genres
    x += tabs_internal_draw( bar, x, y, bar_height, bar->genres_text, bar->genres_width, i == bar->highlighted_tab_index,
        tabs_active[ i ], tabs_active[ i - 1 ], tabs_active[ i + 1 ], leftmost, not_rightmost );
    ++i;
    
    // Artists
    x += tabs_internal_draw( bar, x, y, bar_height, bar->artists_text, bar->artists_width, i == bar->highlighted_tab_index,
        tabs_active[ i ], tabs_active[ i - 1 ], tabs_active[ i + 1 ], not_leftmost, not_rightmost );
    ++i;

    // Albums
    x += tabs_internal_draw( bar, x, y, bar_height, bar->albums_text, bar->albums_width, i == bar->highlighted_tab_index,
        tabs_active[ i ], tabs_active[ i - 1 ], tabs_active[ i + 1 ], not_leftmost, not_rightmost );
    ++i;

    // Tracks
    x += tabs_internal_draw( bar, x, y, bar_height, bar->tracks_text, bar->tracks_width, i == bar->highlighted_tab_index,
        tabs_active[ i ], tabs_active[ i - 1 ], tabs_active[ i + 1 ], not_leftmost, not_rightmost );
    ++i;

    // Tracks
    bool rightval = rightmost;
    x += tabs_internal_draw( bar, x, y, bar_height, bar->shuffle_text, bar->shuffle_width, i == bar->highlighted_tab_index,
        tabs_active[ i ], tabs_active[ i - 1 ], tabs_active[ i + 1 ], not_leftmost, rightval );
    ++i;

    // righthand filler
    x += bitmap_draw_tab_tiled( bar->render, bar->bmp_notab_shadow, bar->bmp_notab_highlight, x, y, bar_height, bar->render->logical_width - x, 0xffffffff );


//     edit_draw( bar->render, bar->search_box, 0xffffffff );

    button_draw( &bar->power, text_col, text_col_highlighted );
    button_draw( &bar->speaker, text_col, text_col_highlighted );
    slider_draw( &bar->volume, text_col, text_col_highlighted );

    if( bar->play_control->album_id != MUSICDB_INVALID_ID && !bar->showing_current_album )
        button_draw( &bar->ctl_playing, text_col, text_col_highlighted );
    button_draw( &bar->ctl_prev, text_col, text_col_highlighted );   
    button_draw( &bar->ctl_pause, text_col, text_col_highlighted );
    button_draw( &bar->ctl_next, text_col, text_col_highlighted );
    if( bar->active_tab_index == 4 ||bar->active_tab_index == 2 ) {
        button_draw( &bar->ctl_shuffle_add, text_col, text_col_highlighted );
        //button_draw( &bar->ctl_shuffle_remove, text_col, text_col_highlighted );
    }
    }

