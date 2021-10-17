

typedef struct screen_albums_item_t
    {
    uint32_t album_id;

    text_t* title_text;
    char title_string[ 256 ];
        
    text_t* artist_text;
    char artist_string[ 256 ];
    char artist_sort[ 256 ];
        
    text_t* track_count_text;
    char track_count_string[ 64 ];

    bitmap_t* bitmap;
    uint32_t thumb_id;
    } screen_albums_item_t;


typedef struct screen_albums_t
    {
    app_t* app;
    render_t* render;
    font_t* font_artist;
    font_t* font_artist_small;
    font_t* font_title;
    font_t* font_title_small;
    font_t* font_tracks;
    font_t* font_filter;
    bitmap_t* frame;
    bitmap_t* album_default;
    int width;
    int height;
    int scroll;
    int highlighted;
    int thumbs_cooldown;

    button_t close_artist;

    int layout_size;
    int layout_hmargin;
    int layout_hspacing;
    int layout_vspacing;
    int layout_hcount;
    int layout_vcount;
    int layout_offset;
    int layout_max_scroll;

    bool force_resize;

    uint32_t current_genre_id;
    uint32_t current_artist_id;

    text_t* filter_text;

    int items_count;
    screen_albums_item_t* items;

    text_t* quicknav[ 28 ];
    int quicknav_hover;
    } screen_albums_t;



bool screen_albums_init( screen_albums_t* screen, app_t* app, render_t* render, bitmap_t* albumdefault, bitmap_t* frame )
    {
    memset( screen, 0, sizeof( *screen ) );
    screen->app = app;
    screen->render = render;
    screen->frame = frame;
    screen->album_default = albumdefault;

    if( !( screen->font_artist = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 32 ) ) ) return false;
    if( !( screen->font_artist_small = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 25 ) ) ) return false;
    if( !( screen->font_title = font_create( screen->render, "gui/fonts/AlegreyaSans-Bold.ttf", 27 ) ) ) return false;
    if( !( screen->font_title_small = font_create( screen->render, "gui/fonts/AlegreyaSans-Bold.ttf", 22 ) ) ) return false;
    if( !( screen->font_tracks = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 22 ) ) ) return false;
    if( !( screen->font_filter = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 38 ) ) ) return false;

    screen->width = (int)render->logical_width;
    screen->height = (int)render->logical_height;
    screen->scroll = 0;
    screen->highlighted = -1;
    screen->thumbs_cooldown = 0;

    button_init( &screen->close_artist, render, load_app_bitmap( render, "gui/icons/close.png", ".cache/gui/icons/close.mip", global_max_w, global_max_h ) );

    screen->current_genre_id = MUSICDB_INVALID_ID;
    screen->current_artist_id = MUSICDB_INVALID_ID;

    screen->items_count = 0;
    screen->items = (screen_albums_item_t*) malloc( sizeof( *screen->items ) * 1 );

    screen->layout_size = 250;
    screen->layout_hmargin = 25;
    screen->layout_hspacing = 25;
    screen->layout_vspacing = 110;
    screen->layout_hcount = (int)( ( screen->render->logical_width - screen->layout_hmargin * 2 ) / ( screen->layout_size + screen->layout_hspacing * 2 ) );
    if( screen->layout_hcount < 1 ) screen->layout_hcount = 1;
    screen->layout_vcount = (int)( ( screen->render->logical_height - 100 ) / ( screen->layout_size + screen->layout_vspacing ) );
    screen->layout_offset = (int)( ( screen->render->logical_width - screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) ) / 2 );
    int padded_item_count = ( screen->items_count % screen->layout_hcount ) 
        ? ( ( ( screen->items_count / screen->layout_hcount ) + 1 ) * screen->layout_hcount ) 
        : ( ( screen->items_count / screen->layout_hcount ) * screen->layout_hcount );
    screen->layout_max_scroll = 3 * ( padded_item_count / screen->layout_hcount - screen->layout_vcount ) + 1;
    if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;
    
    for( int i = 0; i < 28; ++i ) 
        {
        char str[2] = "#";
        if( i == 27 ) *str = '?';
        else if( i > 0 ) *str = (char)( 'A' + ( i - 1 ) );
        screen->quicknav[ i ] = text_create( render, screen->font_artist, str );
        }
    screen->quicknav_hover = -1;
    
    return true;
    }


void screen_albums_term( screen_albums_t* screen )
    {
    for( int i = 0; i < 28; ++i ) 
        {
        text_destroy( screen->render, screen->quicknav[ i ] );
        }
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_albums_item_t* item = &screen->items[ i ];
        if( item->bitmap ) bitmap_destroy( screen->render, item->bitmap );
        if( item->title_text ) text_destroy( screen->render, item->title_text );
        if( item->artist_text ) text_destroy( screen->render, item->artist_text );
        if( item->track_count_text ) text_destroy( screen->render, item->track_count_text );
        }
    if( screen->filter_text ) text_destroy( screen->render, screen->filter_text );
    button_term( &screen->close_artist );
    font_destroy( screen->render, screen->font_filter );
    font_destroy( screen->render, screen->font_tracks );
    font_destroy( screen->render, screen->font_title_small );
    font_destroy( screen->render, screen->font_title );
    font_destroy( screen->render, screen->font_artist_small );
    font_destroy( screen->render, screen->font_artist );
    free( screen->items );
    }


void screen_albums_set_genre( screen_albums_t* screen, uint32_t genre_id )
    {
    if( screen->current_genre_id != genre_id || screen->current_artist_id != MUSICDB_INVALID_ID )
        {
        screen->scroll = 0;
        screen->current_genre_id = genre_id;
        screen->current_artist_id = MUSICDB_INVALID_ID;
        if( screen->filter_text ) text_destroy( screen->render, screen->filter_text );
        screen->filter_text = 0;
        screen->force_resize = true;
        }
    }

void screen_albums_set_artist( screen_albums_t* screen, uint32_t artist_id )
    {
    if( screen->current_artist_id != artist_id || screen->current_genre_id != MUSICDB_INVALID_ID)
        {
        screen->scroll = 0;
        screen->current_artist_id = artist_id;
        screen->current_genre_id = MUSICDB_INVALID_ID;
        if( screen->filter_text ) text_destroy( screen->render, screen->filter_text );
        screen->filter_text = 0;
        screen->force_resize = true;
        }
    }

void screen_albums_set_various_artists( screen_albums_t* screen, uint32_t artist_id, uint32_t genre_id )
    {
    if( screen->current_artist_id != artist_id || screen->current_genre_id != genre_id )
        {
        screen->scroll = 0;
        screen->current_artist_id = artist_id;
        screen->current_genre_id = genre_id;
        if( screen->filter_text ) text_destroy( screen->render, screen->filter_text );
        screen->filter_text = 0;
        screen->force_resize = true;
        }
    }


void screen_albums_refresh( screen_albums_t* screen, music_db_t* musicdb, bool remap_genre )
    {
    int scroll_index = ( screen->scroll / 3 ) * screen->layout_hcount;
    int scroll_offset = screen->scroll % 3;
    char scroll_item_title[ 256 ] = "";
    char scroll_item_artist[ 256 ] = "";
    if( scroll_index > 0 && scroll_index < screen->items_count)
        {
        strcpy( scroll_item_title, screen->items[ scroll_index ].title_string );
        strcpy( scroll_item_artist, screen->items[ scroll_index ].artist_string );
        }

    if( screen->current_artist_id != MUSICDB_INVALID_ID && screen->current_genre_id == MUSICDB_INVALID_ID) remap_genre = false;

    int albums_count = 0;
    musicdb_album_t* albums = musicdb_albums_get( musicdb, screen->current_genre_id, screen->current_artist_id, remap_genre, &albums_count );

    screen_albums_item_t* new_items = (screen_albums_item_t*) malloc( sizeof( *new_items ) * albums_count );

    int new_items_count = 0;


    char str[ 256 ];
    for( int i = 0; i < albums_count; ++i )
        {
        musicdb_album_t* album = &albums[ i ];
        bool found = false;
        for( int j = 0; j < screen->items_count; ++j )
            {
            if( screen->items[ j ].album_id == album->id )
                {
                screen_albums_item_t* item = &new_items[ new_items_count++ ];
                *item = screen->items[ j ];
                memset( &screen->items[ j ], 0, sizeof( screen_albums_item_t ) );
                screen->items[ j ].album_id = MUSICDB_INVALID_ID;

                if( album->compilation )
                    sprintf( str, "Compilation  (%d songs)", album->track_count );
                else if( album->year )
                    sprintf( str, "%d  (%d songs)", album->year, album->track_count );
                else
                    sprintf( str, "(%d songs)", album->track_count );
                
                if( strcmp( item->track_count_string, str ) != 0 )
                    {
                    if( item->track_count_text ) text_destroy( screen->render, item->track_count_text );
                    item->track_count_text = 0;
                    strcpy( item->track_count_string, str );
                    }

                if( album->thumb_id != item->thumb_id )
                    {
                    item->thumb_id = album->thumb_id;
                    if( item->bitmap ) bitmap_destroy( screen->render, item->bitmap );
                    item->bitmap = NULL;
                    }
                found = true;
                break;
                }
            }
        if( !found )
            {
            screen_albums_item_t* item = &new_items[ new_items_count++ ];

            item->album_id = album->id;

            item->title_text = 0;
            strncpy( str, album->title, sizeof( str ) - 4 );
            str[ sizeof( str ) - 4 ] = '\0';        
            strcpy( item->title_string, str );
        
            item->artist_text = 0;
            strncpy( str, album->artist, sizeof( str ) - 4 );
            str[ sizeof( str ) - 4 ] = '\0';   
            strcpy( item->artist_string, str );

            strncpy( str, *album->sort_artist ? album->sort_artist : album->artist, sizeof( str ) - 4 );
            str[ sizeof( str ) - 4 ] = '\0';   
            strcpy( item->artist_sort, str );
        
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

    screen->layout_max_scroll = 3 * ( screen->items_count / screen->layout_hcount - ( screen->layout_vcount - 1 ) ) - 1;
    if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;

    scroll_index = 0;
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_albums_item_t* item = &screen->items[ i ];
        if( stricmp( item->title_string, scroll_item_title ) == 0 && stricmp( item->artist_string, scroll_item_artist ) == 0 )
            scroll_index = i;
        }
    screen->scroll = scroll_offset + 3 * ( scroll_index / screen->layout_hcount );

    musicdb_albums_release( musicdb, albums );
    screen->force_resize = true;
    }


typedef struct screen_albums_navigation_t
    {
    screen_t screen;
    uint32_t selected_album_id;
    uint32_t selected_artist_id;
    uint32_t selected_genre_id;
    } screen_albums_navigation_t;

bool screen_albums_update( screen_albums_t* screen, screen_albums_navigation_t* navigation, bool resize, input_t const* input )
    {
    bool redraw = false;
    navigation->screen = SCREEN_ALBUMS;
    navigation->selected_album_id = MUSICDB_INVALID_ID;
    navigation->selected_artist_id = screen->current_artist_id;
    navigation->selected_genre_id = screen->current_genre_id;

    int offset = 0;    
    if( screen->filter_text ) offset = 45;

    if( resize || screen->force_resize || screen->width != (int)screen->render->logical_width || screen->height != (int)screen->render->logical_height )
        {
        screen->force_resize = false;
        
        screen->width = (int)screen->render->logical_width;
        screen->height = (int)screen->render->logical_height;
        screen->highlighted = -1;

        int scroll_item = ( screen->scroll / 3 ) * screen->layout_hcount;
        int scroll_offset = screen->scroll % 3;

        screen->layout_hcount = (int)( ( screen->render->logical_width - screen->layout_hmargin * 2 ) / ( screen->layout_size + screen->layout_hspacing * 2 ) );
        if( screen->layout_hcount < 1 ) screen->layout_hcount = 1;
        screen->layout_vcount = (int)( ( screen->render->logical_height - 100 ) / ( screen->layout_size + screen->layout_vspacing ) );
        screen->layout_offset = (int)( ( screen->render->logical_width - screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) ) / 2 );

        int row_count = ( screen->items_count + ( screen->layout_hcount - 1 ) ) / screen->layout_hcount;
        int scroll_end = row_count * ( screen->layout_size + screen->layout_vspacing );
        int bottom_space = screen->current_artist_id == MUSICDB_INVALID_ID ? 44 : 0;
        int screen_steps = ( (int)screen->render->logical_height - ( 125 + bottom_space ) ) / ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
        int scroll_steps = ( 3 * scroll_end ) / ( screen->layout_size + screen->layout_vspacing );
        screen->layout_max_scroll = scroll_steps - screen_steps;
        if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;
        screen->scroll = scroll_offset + 3 * ( scroll_item / screen->layout_hcount );
        if( screen->scroll > screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;

        for( int i = 0; i < screen->items_count; ++i )
            {
            screen_albums_item_t* item = &screen->items[ i ];
            if( item->title_text ) text_destroy( screen->render, item->title_text );
            item->title_text = 0;
            if( item->artist_text ) text_destroy( screen->render, item->artist_text );
            item->artist_text = 0;
            if( item->track_count_text ) text_destroy( screen->render, item->track_count_text );
            item->track_count_text = 0;
            }

        if( screen->filter_text ) text_destroy( screen->render, screen->filter_text );
        screen->filter_text = 0;

        for( int i = 0; i < 28; ++i ) 
            {
            char str[2] = "#";
            if( i == 27 ) *str = '?';
            else if( i > 0 ) *str = (char)( 'A' + ( i - 1 ) );
            text_destroy( screen->render, screen->quicknav[ i ] );
            screen->quicknav[ i ] = text_create( screen->render, screen->font_artist, str );
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

    bool hover = false;
    redraw |= button_update( &screen->close_artist, input->mouse_x, input->mouse_y, &hover );
    if( hover && input->click )
        {
        navigation->screen = SCREEN_ALBUMS;
        navigation->selected_artist_id = MUSICDB_INVALID_ID;
        navigation->selected_genre_id = MUSICDB_INVALID_ID;
        }



    int highlighted = -1;
    int x = 0;
    int y = offset -screen->scroll * ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
    for( int i = 0; i < screen->items_count; ++i )
        {
        if( input->mouse_x >= screen->layout_hspacing + x + screen->layout_offset - 8 && 
            input->mouse_x < screen->layout_hspacing + x + screen->layout_offset + screen->layout_size + 8 && 
            input->mouse_y > offset + 60 && input->mouse_y >= y + 100 - 8 && input->mouse_y < y + 100 + screen->layout_size + 60 + 8 &&
            ( screen->current_artist_id != MUSICDB_INVALID_ID || input->mouse_y < (int)screen->render->logical_height - 44 )
            )
            {
            highlighted = i;
            if( input->click )
                {
                if( input->click_start_x >= screen->layout_hspacing + x + screen->layout_offset - 8 && 
                    input->click_start_x < screen->layout_hspacing + x + screen->layout_offset + screen->layout_size + 8 && 
                    input->click_start_y > offset + 60 && input->click_start_y >= y + 100 - 8 && input->click_start_y < y + 100 + screen->layout_size + 60 + 8 &&
                    ( screen->current_artist_id != MUSICDB_INVALID_ID || input->mouse_y < (int)screen->render->logical_height - 44 )
                    )
                    {
                    navigation->screen = SCREEN_TRACKS;
                    navigation->selected_album_id = screen->items[ i ].album_id;
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

    if( highlighted != screen->highlighted )
        {
        screen->highlighted = highlighted;
        redraw = true;
        }

    if( screen->current_artist_id == MUSICDB_INVALID_ID )
        {
        int quicknav_hover = -1;
        for( int i = 0; i < 28; ++i ) 
            {
            int xpos = ( (int)screen->render->logical_width - 40 * 28 ) / 2;
            xpos += 40 * i;
            int ypos = (int)screen->render->logical_height - 10;
            if( input->mouse_x >= xpos && input->mouse_x < xpos + 40 && input->mouse_y > ypos - 30 ) 
                {
                quicknav_hover = i;
                break;
                }
            }

        if( quicknav_hover != screen->quicknav_hover )
            {
            screen->quicknav_hover = quicknav_hover;
            redraw = true;
            }
            
        bool letter = false;
        for( int i = 0; i < 26; ++i )
            {
            if( input->letters_a_to_z[ i ] ) 
                {
                quicknav_hover = 1 + i;
                letter = true;
                break;
                }
            }

        if( input->zero )
            {
            quicknav_hover = 0;
            letter = true;
            }

        if( input->question )
            {
            quicknav_hover = 27;
            letter = true;
            }

        if( quicknav_hover >= 0 && ( input->click || letter ) )
            {
            int scroll_item = 0;
            if( quicknav_hover == 27 )
                {
                for( int i = 0; i < 20; ++i) 
                    {
                    scroll_item = random( 0, screen->items_count - 1 );
                    int screen_scroll = 3 * ( scroll_item / screen->layout_hcount );
                    if( screen_scroll >= screen->layout_max_scroll ) screen_scroll = screen->layout_max_scroll;
                    if( screen_scroll != screen->scroll ) break;
                    }
                }
            else if( quicknav_hover > 0 ) 
                {
                int s = 0;
                if( strnicmp( screen->items[ 0 ].artist_string, "All ",4 ) == 0 ) ++s;
                for( int i = s; i < screen->items_count; ++i )
                    {
                    char nav = (char)( 'A' + ( quicknav_hover - 1 ) );
                    const char* artiststr = screen->items[ i ].artist_sort;
                    if( stricmp( artiststr, "Various Artists" ) == 0 ) break;
                    if( strnicmp( artiststr, "The ", 4 ) == 0 ) artiststr +=4;
                    char artist = (char) toupper( *artiststr );
                    if( artist > nav ) break;
                    scroll_item = i;
                    if( artist == nav )break;
                    }
                }
            screen->scroll = 3 * ( scroll_item / screen->layout_hcount );
            if( screen->scroll >= screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;
            redraw = true;
            }
        }        
    return redraw;
    }



bool screen_albums_draw( screen_albums_t* screen, music_db_t* musicdb, bool remap_genre )
    {
    if( !screen->filter_text )
        {
        if( screen->current_artist_id != MUSICDB_INVALID_ID )
            {
            musicdb_artist_t artist_info = { 0 };
            musicdb_artist_info( musicdb, screen->current_artist_id, &artist_info );
            char str[ 256 ];
            strcpy( str, artist_info.name );
            if( stricmp( str, "Various Artists" ) == 0 && screen->current_genre_id != MUSICDB_INVALID_ID ) 
                {
                musicdb_genre_t genre_info = { 0 };
                musicdb_genre_info( musicdb, screen->current_genre_id, &genre_info );
                strcat( str, ": " );
                strcat( str, remap_genre ? genre_info.remapped_label : genre_info.label );
                }
            screen->filter_text = text_create( screen->render, screen->font_filter, str );      
            }
        else if( screen->current_genre_id != MUSICDB_INVALID_ID )
            {
            musicdb_genre_t genre_info = { 0 };
            musicdb_genre_info( musicdb, screen->current_genre_id, &genre_info );
            screen->filter_text = text_create( screen->render, screen->font_filter, remap_genre ? genre_info.remapped_label : genre_info.label );      
            }
        }
    
    int offset = 0;    
    if( screen->filter_text )
        {
        text_draw( screen->render, screen->filter_text, screen->layout_hspacing + screen->layout_offset, 110, 0xffffffff );
        button_size( &screen->close_artist, screen->layout_hspacing + screen->layout_offset - 38, 81, 36, 36 );
        button_draw( &screen->close_artist, 0xff736d64, 0xffd98b1a );   
        offset = 45;
        }

    int space = 0;
    if( screen->current_artist_id == MUSICDB_INVALID_ID ) space = 44;
    render_scissor( screen->render, 0, 80 + offset, (int)screen->render->logical_width, (int)screen->render->logical_height - space );
    int scroll_index = ( screen->scroll / 3  - 1 ) * screen->layout_hcount;
    int x = 0;
    int y = offset -screen->scroll * ( ( screen->layout_size + screen->layout_vspacing ) / 3 );

    char str[ 256 ];
    int end = screen->items_count;
    for( int i = 0; i < screen->items_count; ++i )
        {        
        screen_albums_item_t* item = &screen->items[ i ];
        if( i >= scroll_index )
            {
            if( !item->bitmap && item->album_id != MUSICDB_INVALID_ID )
                {
                int w, h, mip_width, mip_height;
                bitmap_t* thumb = musicdb_get_album_thumbnail( musicdb, item->album_id, &w, &h, &mip_width, &mip_height, screen->render );
                if( thumb )
                    {
                    item->bitmap = thumb;
                    if( ! item->bitmap ) 
                        item->album_id = MUSICDB_INVALID_ID;
                    }
                }
            if( !item->title_text )
                {
                font_t* font_title = screen->font_title;
                strcpy( str, item->title_string );
                if( text_width( screen->render, screen->font_title, str ) > screen->layout_size + screen->layout_hmargin )
                    {
                    font_title = screen->font_title_small;
                    while( strlen( str ) > 5 && text_width( screen->render, screen->font_title_small, str ) > screen->layout_size + screen->layout_hmargin )
                        {
                        char* ptr = str + strlen( str ) - 1;
                        while( ptr > str && !isalnum( (uint8_t) *ptr ) )
                            {
                            --ptr;
                            }
                        *ptr = '\0';
                        strcat( str, "..." );
                        }
                    }
                item->title_text = text_create( screen->render, font_title, str );
                }
        
            if( !item->artist_text )
                {
                strcpy( str, item->artist_string );
                font_t* font_artist = screen->font_artist;
                if( text_width( screen->render, screen->font_artist, str ) > screen->layout_size + screen->layout_hmargin )
                    {
                    font_artist = screen->font_artist_small;
                    while( strlen( str ) > 5 && text_width( screen->render, screen->font_artist_small, str ) > screen->layout_size + screen->layout_hmargin )
                        {
                        char* ptr = str + strlen( str ) - 1;
                        while( ptr > str && !isalnum( (uint8_t) *ptr ) )
                            {
                            --ptr;
                            }
                        *ptr = '\0';
                        strcat( str, "..." );
                        }
                    }
                item->artist_text = text_create( screen->render, font_artist, str );
                }        

            if( !item->track_count_text )
                {
                strcpy( str, item->track_count_string );
                item->track_count_text = text_create( screen->render, screen->font_tracks, str );      
                }

            bitmap_draw( screen->render, item->bitmap ? item->bitmap : screen->album_default, 25 + x + screen->layout_offset, y + 100, screen->layout_size, screen->layout_size, 0xffffffff );
            uint32_t col = 0xffffffff;
            if( screen->highlighted == i )
                {
                col = 0xffd98b1a;
                bitmap_draw( screen->render, screen->frame, screen->layout_hspacing + x + screen->layout_offset - 8, y + 100 - 8, 266, 266, col );
                }
            if( item->artist_text ) text_draw( screen->render, item->artist_text, screen->layout_hspacing + x + ( screen->layout_size - ( item->artist_text->width > screen->layout_size + screen->layout_hspacing ? screen->layout_size + screen->layout_hspacing: item->artist_text->width ) ) / 2 + screen->layout_offset, y + 100 + screen->layout_size + 34, col );
            if( item->title_text ) text_draw( screen->render, item->title_text, screen->layout_hspacing + x + ( screen->layout_size - ( item->title_text->width > screen->layout_size + screen->layout_hspacing ? screen->layout_size + screen->layout_hspacing : item->title_text->width ) ) / 2 + screen->layout_offset, y + 100 + screen->layout_size + 59, 0xffd98b1a );
            if( item->track_count_text ) text_draw( screen->render, item->track_count_text, screen->layout_hspacing + x + ( screen->layout_size - item->track_count_text->width ) / 2 + screen->layout_offset, y + 100 + screen->layout_size + 80, 0xff736d64 );
            }
        else
            {
            if( item->bitmap ) bitmap_destroy( screen->render, item->bitmap );
            if( item->artist_text ) text_destroy( screen->render, item->artist_text );
            if( item->title_text ) text_destroy( screen->render,item->title_text );
            if( item->track_count_text ) text_destroy( screen->render,item->track_count_text );
            item->bitmap = 0;
            item->artist_text = 0;
            item->title_text = 0;
            item->track_count_text = 0;
            }
        x += screen->layout_size + screen->layout_hspacing * 2;
        if( x >= screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) )
            {
            x = 0;
            y += screen->layout_size + screen->layout_vspacing;
            }
        
        if( y > screen->height ) { end = i; break; }
        }

    for( int i = end + 1; i < screen->items_count; ++i )
        {
        screen_albums_item_t* item = &screen->items[ i ];
        if( item->bitmap ) bitmap_destroy( screen->render, item->bitmap );
        if( item->artist_text ) text_destroy( screen->render, item->artist_text );
        if( item->title_text ) text_destroy( screen->render,item->title_text );
        if( item->track_count_text ) text_destroy( screen->render,item->track_count_text );
        item->bitmap = 0;
        item->artist_text = 0;
        item->title_text = 0;
        item->track_count_text = 0;
        }

    render_scissor_off( screen->render );

    if( screen->current_artist_id == MUSICDB_INVALID_ID )
        {
        for( int i = 0; i < 28; ++i ) 
            {
            text_t* text = screen->quicknav[ i ];
            int width = text_width( screen->render, screen->font_artist, text->string );
            int xpos = ( (int)screen->render->logical_width - 40 * 28 ) / 2;
            xpos += 40 * i + ( 40 - width ) / 2;
            int ypos = (int)screen->render->logical_height - 10;
            uint32_t color = i == screen->quicknav_hover ? 0xffd98b1a : 0xff736d64;
            text_draw( screen->render, text, xpos, ypos, color );
            }
        }

    return false;
    }

