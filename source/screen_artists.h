

typedef struct screen_artists_item_t
    {
    uint32_t artist_id;
    uint32_t single_album_id;

    char artist_string[ 256 ];
    text_t* artist_text;

    char album_count_string[ 256 ];
    text_t* album_count_text;

    uint32_t thumb_id;
    bitmap_t* bitmap;
    } screen_artists_item_t;


typedef struct screen_artists_t
    {
    app_t* app;
    render_t* render;
    font_t* font_artist;
    font_t* font_artist_small;
    font_t* font_albums;
    font_t* font_filter;
    bitmap_t* variousartists;
    bitmap_t* allalbums;
    bitmap_t* genredefault;
    bitmap_t* frame;
    int width;
    int height;
    int scroll;
    int highlighted;

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
    uint32_t current_genre_thumb;

    text_t* filter_text;

    int items_count;
    screen_artists_item_t* items;

    text_t* quicknav[ 28 ];
    int quicknav_hover;
    } screen_artists_t;


bool screen_artists_init( screen_artists_t* screen, app_t* app, render_t* render, bitmap_t* genredefault, bitmap_t* allalbums, bitmap_t* variousartists, bitmap_t* frame )
    {
    memset( screen, 0, sizeof( *screen ) );
    screen->app = app;
    screen->render = render;
    screen->allalbums = allalbums;
    screen->variousartists = variousartists;
    screen->genredefault = genredefault;
    screen->frame = frame;
    if( !( screen->font_artist = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 32 ) ) ) return false;
    if( !( screen->font_artist_small = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 25 ) ) ) return false;
    if( !( screen->font_albums = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 24 ) ) ) return false;
    if( !( screen->font_filter = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 38 ) ) ) return false;

    screen->width = (int)render->logical_width;
    screen->height = (int)render->logical_height;
    screen->scroll = 0;
    screen->highlighted = -1;

    screen->current_genre_id = MUSICDB_INVALID_ID;
    screen->current_genre_thumb = MUSICDB_INVALID_ID;

    button_init( &screen->close_artist, render, load_app_bitmap( render, "gui/icons/close.png", ".cache/gui/icons/close.mip", global_max_w, global_max_h ) );

    screen->items_count = 0;
    screen->items = (screen_artists_item_t*) malloc( sizeof( *screen->items ) * 1 );

    screen->layout_size = 250;
    screen->layout_hmargin = 25;
    screen->layout_hspacing = 25;
    screen->layout_vspacing = 90;
    screen->layout_hcount = (int)( ( screen->render->logical_width - screen->layout_hmargin * 2 ) / ( screen->layout_size + screen->layout_hspacing * 2 ) );
    if( screen->layout_hcount < 1 ) screen->layout_hcount = 1;
    screen->layout_vcount = (int)( ( screen->render->logical_height - 100 ) / ( screen->layout_size + screen->layout_vspacing ) );
    screen->layout_offset = (int)( ( screen->render->logical_width - screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) ) / 2 );
    int padded_item_count = ( screen->items_count % screen->layout_hcount ) 
        ? ( ( ( screen->items_count / screen->layout_hcount ) + 1 ) * screen->layout_hcount ) 
        : ( ( screen->items_count / screen->layout_hcount ) * screen->layout_hcount );
    screen->layout_max_scroll = 3 * ( padded_item_count / screen->layout_hcount - screen->layout_vcount ) + 1;
    if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;

    screen_artists_item_t* all_albums = &screen->items[ screen->items_count++ ];
    all_albums->artist_id = MUSICDB_INVALID_ID;
    all_albums->single_album_id = MUSICDB_INVALID_ID;
    strcpy( all_albums->artist_string, "All albums" );
    all_albums->artist_text = 0;
    *all_albums->album_count_string = '\0';
    all_albums->album_count_text = 0;
    all_albums->thumb_id = MUSICDB_INVALID_ID;
    all_albums->bitmap = screen->allalbums;

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


void screen_artists_term( screen_artists_t* screen )
    {
    for( int i = 0; i < 28; ++i ) 
        {
        text_destroy( screen->render, screen->quicknav[ i ] );
        }
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_artists_item_t* item = &screen->items[ i ];
        if( item->bitmap && item->bitmap != screen->variousartists && item->bitmap != screen->allalbums ) bitmap_destroy( screen->render, item->bitmap );
        if( item->artist_text ) text_destroy( screen->render, item->artist_text );
        if( item->album_count_text ) text_destroy( screen->render, item->album_count_text );
        }
    if( screen->filter_text ) text_destroy( screen->render, screen->filter_text );
    button_term( &screen->close_artist );
    font_destroy( screen->render, screen->font_filter );
    font_destroy( screen->render, screen->font_albums );
    font_destroy( screen->render, screen->font_artist_small );
    font_destroy( screen->render, screen->font_artist );
    free( screen->items );
    }


void screen_artists_set_genre( screen_artists_t* screen, uint32_t genre_id, uint32_t thumb_id )
    {
    if( screen->current_genre_id != genre_id  )
        {
        screen->scroll = 0;
        screen->current_genre_id = genre_id;
        screen->current_genre_thumb = thumb_id;
        if( screen->filter_text ) text_destroy( screen->render, screen->filter_text );
        screen->filter_text = 0;
        screen->force_resize = true;
        }
    }


void screen_artists_refresh( screen_artists_t* screen, music_db_t* musicdb, bool remap_genre )
    {
    int scroll_index = ( screen->scroll / 3 ) * screen->layout_hcount;
    int scroll_offset = screen->scroll % 3;
    char scroll_item[ 128 ] = "";
    if( scroll_index > 0 && scroll_index < screen->items_count )
        strcpy( scroll_item, screen->items[ scroll_index ].artist_string );

    int artists_count = 0;
    int total_album_count = 0;
    musicdb_artist_t* artists = musicdb_artists_get( musicdb, screen->current_genre_id, remap_genre, &artists_count, &total_album_count );

    screen_artists_item_t* new_items = (screen_artists_item_t*) malloc( sizeof( *new_items ) * ( artists_count + 1 ) );
    int new_items_count = 0;

    char str[ 256 ];
    new_items[ new_items_count++ ] = screen->items[ 0 ];
    memset( &screen->items[ 0 ], 0, sizeof( screen_artists_item_t ) );
    screen->items[ 0 ].artist_id = MUSICDB_INVALID_ID;
    screen->items[ 0 ].single_album_id = MUSICDB_INVALID_ID;

    if( screen->current_genre_id == MUSICDB_INVALID_ID )
        {
        screen_artists_item_t* all_albums = &new_items[ 0 ];
        all_albums->artist_id = MUSICDB_INVALID_ID;
        all_albums->single_album_id = MUSICDB_INVALID_ID;
        strcpy( all_albums->artist_string, "All albums" );
        if( all_albums->artist_text ) text_destroy( screen->render, all_albums->artist_text );
        all_albums->artist_text = 0;
        *all_albums->album_count_string = '\0';
        all_albums->thumb_id = MUSICDB_INVALID_ID;
        if( all_albums->bitmap && all_albums->bitmap != screen->allalbums ) bitmap_destroy( screen->render, all_albums->bitmap );
        all_albums->bitmap = screen->allalbums;
        sprintf( str, "%d albums", total_album_count );
        if( all_albums->album_count_text ) text_destroy( screen->render, all_albums->album_count_text );
        all_albums->album_count_text = 0;
        strcpy( all_albums->album_count_string, str );
        }
    else
        {
        musicdb_genre_t genre_info = { 0 };
        musicdb_genre_info( musicdb, screen->current_genre_id, &genre_info );

        screen_artists_item_t* genre_albums = &new_items[ 0 ];
        genre_albums->artist_id = MUSICDB_INVALID_ID;
        genre_albums->single_album_id = MUSICDB_INVALID_ID;
        sprintf( str, "All %s albums", remap_genre ? genre_info.remapped_label : genre_info.label );
        strcpy( genre_albums->artist_string, str );
        if( genre_albums->artist_text ) text_destroy( screen->render, genre_albums->artist_text );
        genre_albums->artist_text = 0;
        *genre_albums->album_count_string = '\0';
        if( genre_albums->album_count_text ) text_destroy( screen->render, genre_albums->album_count_text );
        genre_albums->album_count_text = 0;
        genre_albums->thumb_id = remap_genre ?  genre_info.remapped_thumb_id : genre_info.thumbnail_id;
        if( genre_albums->bitmap && genre_albums->bitmap != screen->allalbums ) bitmap_destroy( screen->render, genre_albums->bitmap );
        genre_albums->bitmap = 0;
        sprintf( str, "%d albums", total_album_count );
        genre_albums->album_count_text = text_create( screen->render, screen->font_albums, str );
        strcpy( genre_albums->album_count_string, str );
        }
    
    int album_count = 0;
    for( int i = 0; i < artists_count; ++i )
        {
        musicdb_artist_t* artist = &artists[ i ];
        char const* label = artist->name;
        if( strlen( label ) > 200 ) continue;
        bool found = false;
        for( int j = 0; j < screen->items_count; ++j )
            {
            if( strcmp( screen->items[ j ].artist_string, label ) == 0 )
                {
                screen_artists_item_t* item = &new_items[ new_items_count++ ];
                *item = screen->items[ j ];
                memset( &screen->items[ j ], 0, sizeof( screen_artists_item_t ) );
                screen->items[ j ].artist_id = MUSICDB_INVALID_ID;
                screen->items[ j ].single_album_id = MUSICDB_INVALID_ID;

                sprintf( str, "%d albums", artist->album_count );               
                if( stricmp( label, "Various Artists" ) == 0 ) 
                    { 
                    sprintf( str, "%d albums", ( screen->current_genre_id != MUSICDB_INVALID_ID ? total_album_count  - album_count : artist->album_count ) );
                    }
                if( strcmp( item->album_count_string, str ) != 0 )
                    {
                    if( item->album_count_text ) text_destroy( screen->render, item->album_count_text );
                    item->album_count_text = 0;
                    strcpy( item->album_count_string, str );
                    }
                if( item->thumb_id != artist->thumb_id && item->bitmap != screen->allalbums && item->bitmap != screen->variousartists)
                    {
                    int w, h, mip_width, mip_height;
                    bitmap_t* thumb = musicdb_get_artist_thumbnail( musicdb, item->artist_id, &w, &h, &mip_width, &mip_height, screen->render );
                    if( thumb )
                        {
                        if( item->bitmap && item->bitmap != screen->allalbums && item->bitmap != screen->variousartists ) 
                            {
                            bitmap_destroy( screen->render, item->bitmap );
                            item->bitmap = 0;
                            }
                        item->bitmap = thumb;
                        if( !item->bitmap )
                            {
                            item->artist_id = MUSICDB_INVALID_ID;
                            item->single_album_id = MUSICDB_INVALID_ID;
                            }
                        }
                    item->thumb_id = artist->thumb_id;
                    }
                item->single_album_id = MUSICDB_INVALID_ID;
                if( artist->album_count == 1 && stricmp( artist->name, "Various Artists" ) != 0 )
                    {
                    int count = 0;
                    musicdb_album_t* album = musicdb_albums_get( musicdb, MUSICDB_INVALID_ID, artist->id, true, &count );
                    if( count == 1 ) item->single_album_id = album->id;
                    musicdb_albums_release( musicdb, album );
                    }
                found = true;
                break;
                }
            }
        if( !found )
            {
            screen_artists_item_t* item = &new_items[ new_items_count++ ];
            item->artist_id = artist->id;
            item->single_album_id = MUSICDB_INVALID_ID;
            if( artist->album_count == 1 )
                {
                int count = 0;
                musicdb_album_t* album = musicdb_albums_get( musicdb, MUSICDB_INVALID_ID, artist->id, true, &count );
                if( count == 1 ) item->single_album_id = album->id;
                musicdb_albums_release( musicdb, album );
                }

            strcpy( item->artist_string, label );
            item->artist_text = 0;
        
            sprintf( str, "%d albums", artist->album_count );
            strcpy( item->album_count_string, str );
            item->album_count_text = 0;
            item->thumb_id = artist->thumb_id;
            item->bitmap = 0;
            if( stricmp( label, "Various Artists" ) == 0 ) 
                { 
                item->bitmap = screen->variousartists;
                sprintf( str, "%d albums", ( screen->current_genre_id != MUSICDB_INVALID_ID ? total_album_count  - album_count : artist->album_count ) );
                strcpy( item->album_count_string, str );
                }
            }
        album_count += artist->album_count;
        }

    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_artists_item_t* item = &screen->items[ i ];
        if( item->bitmap && item->bitmap != screen->allalbums && item->bitmap != screen->variousartists ) bitmap_destroy( screen->render, item->bitmap );
        if( item->artist_text ) text_destroy( screen->render, item->artist_text );
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
            screen_artists_item_t* item = &screen->items[ i ];
            if( stricmp( item->artist_string, scroll_item ) == 0 )
                scroll_index = i;
            }
        }
    screen->scroll = scroll_offset + 3 * ( scroll_index / screen->layout_hcount );

    musicdb_artists_release( musicdb, artists );

    screen->force_resize = true;
    }


typedef struct screen_artists_navigation_t
    {
    screen_t screen;
    uint32_t selected_artist_id;
    uint32_t selected_album_id;
    uint32_t selected_genre_id;
    uint32_t genre_thumb_id;
    } screen_artists_navigation_t;


bool screen_artists_update( screen_artists_t* screen, screen_artists_navigation_t* navigation, bool resize, input_t const* input )
    {
    bool redraw = false;
    navigation->screen = SCREEN_ARTISTS;
    navigation->selected_artist_id = MUSICDB_INVALID_ID;
    navigation->selected_genre_id = screen->current_genre_id;
    navigation->selected_album_id = MUSICDB_INVALID_ID;
    navigation->genre_thumb_id = screen->current_genre_thumb;

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
        int bottom_space = 44;
        int screen_steps = ( (int)screen->render->logical_height - ( 125 + bottom_space ) ) / ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
        int scroll_steps = ( 3 * scroll_end ) / ( screen->layout_size + screen->layout_vspacing );
        screen->layout_max_scroll = scroll_steps - screen_steps;
        if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;
        screen->scroll = scroll_offset + 3 * ( scroll_item / screen->layout_hcount );
        if( screen->scroll > screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;

        for( int i = 0; i < screen->items_count; ++i )
            {
            screen_artists_item_t* item = &screen->items[ i ];
            if( item->artist_text ) text_destroy( screen->render, item->artist_text );
            item->artist_text = 0;
            if( item->album_count_text ) text_destroy( screen->render, item->album_count_text );
            item->album_count_text = 0;
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
        navigation->screen = SCREEN_ARTISTS;
        navigation->selected_genre_id = MUSICDB_INVALID_ID;        
        }

    int highlighted = -1;
    int x = 0;
    int y = -screen->scroll * ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_artists_item_t* item = &screen->items[ i ];
         if( input->mouse_x >= screen->layout_hspacing + x + screen->layout_offset - 8 && 
             input->mouse_x < screen->layout_hspacing + x + screen->layout_offset + screen->layout_size + 8 && 
             input->mouse_y > 60 && input->mouse_y >= y + 100 - 8 && input->mouse_y < y + 100 + screen->layout_size + 60 + 8 &&
             input->mouse_y < (int)screen->render->logical_height - 44
             )
            {
            highlighted = i;
            if( input->click )
                {
                 if( input->click_start_x >= screen->layout_hspacing + x + screen->layout_offset - 8 && 
                     input->click_start_x < screen->layout_hspacing + x + screen->layout_offset + screen->layout_size + 8 && 
                     input->click_start_y > 60 && input->click_start_y >= y + 100 - 8 && input->click_start_y < y + 100 + screen->layout_size + 60 + 8 &&
                     input->mouse_y < (int)screen->render->logical_height - 44
                     )
                    {
                    if( item->single_album_id != MUSICDB_INVALID_ID )
                        {
                        navigation->screen = SCREEN_TRACKS;
                        navigation->selected_album_id = item->single_album_id;
                        }
                    else
                        {
                        navigation->screen = SCREEN_ALBUMS;
                        navigation->selected_artist_id = item->artist_id;
                        }
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
            for( int i = 1; i < screen->items_count; ++i )
                {
                char nav = (char)( 'A' + ( quicknav_hover - 1 ) );
                const char* artiststr = screen->items[ i ].artist_string;
                if( strnicmp( artiststr, "The ", 4 ) == 0 ) artiststr +=4;
                char artist = (char) toupper( *artiststr );
                if( artist > nav ) break;
                scroll_item = i;
                if( artist == nav ) break;
                }
            }
        screen->scroll = 3 * ( scroll_item / screen->layout_hcount );
        if( screen->scroll >= screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;
        redraw = true;
        }
        
    return redraw;
    }



bool screen_artists_draw( screen_artists_t* screen, music_db_t* musicdb, bool remap_genre )
    {
    if( !screen->filter_text )
        {
        if( screen->current_genre_id != MUSICDB_INVALID_ID )
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

    render_scissor( screen->render, 0, 80 + offset, (int)screen->render->logical_width, (int)screen->render->logical_height - 44 );
    int scroll_index = ( screen->scroll / 3  - 1 ) * screen->layout_hcount;
    int x = 0;
    int y = offset-screen->scroll * ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
    char str[ 256 ];
    int end = screen->items_count;
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_artists_item_t* item = &screen->items[ i ];
        if( i >= scroll_index )
            {

            if( !item->bitmap && item->artist_id == MUSICDB_INVALID_ID )
                {
                int w, h, mip_width, mip_height;
                bitmap_t* thumb = musicdb_get_thumbnail( musicdb, item->thumb_id, &w, &h, &mip_width, &mip_height, screen->render );
                if( thumb )
                    {
                    item->bitmap = thumb;
                    if( !item->bitmap ) 
                        item->artist_id = MUSICDB_INVALID_ID;
                    }
                }
            if( !item->bitmap && item->artist_id != MUSICDB_INVALID_ID )
                {
                int w, h, mip_width, mip_height;
                bitmap_t* thumb = musicdb_get_artist_thumbnail( musicdb, item->artist_id, &w, &h, &mip_width, &mip_height, screen->render );
                if( thumb )
                    {
                    item->bitmap = thumb;
                    if( !item->bitmap ) 
                        item->artist_id = MUSICDB_INVALID_ID;
                    }
                }
            if( !item->artist_text )
                {
                strcpy( str, item->artist_string );
                font_t* font_artist = screen->font_artist;
                if( text_width( screen->render, screen->font_artist, str ) > screen->layout_size + screen->layout_hmargin )
                    {
                    font_artist = screen->font_artist_small;
                    while( strlen( str ) > 5 && text_width( screen->render, screen->font_artist_small, str ) > screen->layout_size + screen->layout_hmargin + 8 )
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


            if( !item->album_count_text ) 
                item->album_count_text = text_create( screen->render, screen->font_albums, item->album_count_string );
        
            bitmap_draw( screen->render, item->bitmap ? item->bitmap : screen->genredefault, 25 + x + screen->layout_offset, y + 100, screen->layout_size, screen->layout_size, 0xffffffff );
            uint32_t col = 0xffffffff;
            if( screen->highlighted == i )
                {
                col = 0xffd98b1a;
                bitmap_draw( screen->render, screen->frame, screen->layout_hspacing + x + screen->layout_offset - 8, y + 100 - 8, 266, 266, col );
                }
            text_draw( screen->render, item->artist_text, screen->layout_hspacing + x + ( screen->layout_size - item->artist_text->width ) / 2 + screen->layout_offset, y + 100 + screen->layout_size + 36, col );
            text_draw( screen->render, item->album_count_text, screen->layout_hspacing + x + ( screen->layout_size - item->album_count_text->width ) / 2 + screen->layout_offset, y + 100 + screen->layout_size + 60, 0xff736d64 );
            }
        else
            {
            if( item->bitmap && item->bitmap != screen->allalbums && item->bitmap != screen->variousartists) 
                { 
                bitmap_destroy( screen->render, item->bitmap );
                item->bitmap = 0;
                }
            if( item->artist_text ) text_destroy( screen->render, item->artist_text );
            if( item->album_count_text ) text_destroy( screen->render,item->album_count_text );            
            item->artist_text = 0;
            item->album_count_text = 0;
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
        screen_artists_item_t* item = &screen->items[ i ];
        if( item->bitmap && item->bitmap != screen->allalbums && item->bitmap != screen->variousartists) 
            {
            bitmap_destroy( screen->render, item->bitmap );
            item->bitmap = 0;
            }
        if( item->artist_text ) text_destroy( screen->render, item->artist_text );
        if( item->album_count_text ) text_destroy( screen->render,item->album_count_text );
        item->artist_text = 0;
        item->album_count_text = 0;
        }
    render_scissor_off( screen->render );

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

    return false;
    }
