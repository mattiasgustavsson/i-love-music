


typedef struct screen_genres_item_t
    {
    uint32_t genre_id;

    char genre_string[ 256 ];
    text_t* genre_text;

    char album_count_string[ 256 ];
    text_t* album_count_text;

    uint32_t thumb_id;
    bitmap_t* bitmap;
    } screen_genres_item_t;


typedef struct screen_genres_t
    {
    app_t* app;
    render_t* render;
    font_t* font_genre;
    font_t* font_albums;
    bitmap_t* allartists;
    bitmap_t* genredefault;
    bitmap_t* frame;
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

    int items_count;
    screen_genres_item_t* items;
    } screen_genres_t;


bool screen_genres_init( screen_genres_t* screen, app_t* app, render_t* render, bitmap_t* genredefault, bitmap_t* allartists, bitmap_t* frame )
    {
    memset( screen, 0, sizeof( *screen ) );
    screen->app = app;
    screen->render = render;
    screen->allartists = allartists;
    screen->genredefault = genredefault;
    screen->frame = frame;
    if( !( screen->font_genre = font_create( screen->render, "gui/fonts/AlegreyaSans-ExtraBold.ttf", 38 ) ) ) return false;
    if( !( screen->font_albums = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 24 ) ) ) return false;

    screen->width = (int)render->logical_width;
    screen->height = (int)render->logical_height;
    screen->scroll = 0;
    screen->highlighted = -1;

    screen->items_count = 0;
    screen->items = (screen_genres_item_t*) malloc( sizeof( *screen->items ) * 1 );

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

    screen_genres_item_t* all_artists = &screen->items[ screen->items_count++ ];
    all_artists->genre_id = MUSICDB_INVALID_ID;
    strcpy( all_artists->genre_string, "All artists" );
    all_artists->genre_text = text_create( screen->render, screen->font_genre, "All artists");   
    *all_artists->album_count_string = '\0';
    all_artists->album_count_text = 0;
    all_artists->thumb_id = MUSICDB_INVALID_ID;
    all_artists->bitmap = screen->allartists;
    
    return true;
    }


void screen_genres_term( screen_genres_t* screen )
    {
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_genres_item_t* item = &screen->items[ i ];
        if( item->bitmap && item->bitmap != screen->allartists ) bitmap_destroy( screen->render, item->bitmap );
        if( item->genre_text ) text_destroy( screen->render, item->genre_text );
        if( item->album_count_text ) text_destroy( screen->render, item->album_count_text );
        }
    font_destroy( screen->render, screen->font_albums );
    font_destroy( screen->render, screen->font_genre );
    free( screen->items );
    }


static int sort_genres( void const* ptr_a, void const* ptr_b )
    { 
    musicdb_genre_t const* a = (musicdb_genre_t const*) ptr_a;
    musicdb_genre_t const* b = (musicdb_genre_t const*) ptr_b;
    return stricmp( a->label, b->label ); 
    } 
        
static int sort_genres_remapped( void const* ptr_a, void const* ptr_b )
    { 
    musicdb_genre_t const* a = (musicdb_genre_t const*) ptr_a;
    musicdb_genre_t const* b = (musicdb_genre_t const*) ptr_b;
    return stricmp( a->remapped_label, b->remapped_label ); 
    } 


bool screen_genres_update_thumbs( screen_genres_t* screen, music_db_t* musicdb )
    {
    bool result = false;
    int pic_count = 0;
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_genres_item_t* item = &screen->items[ i ];
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


void screen_genres_refresh( screen_genres_t* screen, music_db_t* musicdb, bool remap_genre )
    {
    int scroll_index = ( screen->scroll / 3 ) * screen->layout_hcount;
    int scroll_offset = screen->scroll % 3;
    char scroll_item[ 128 ] = "";
    if( scroll_index > 0 && scroll_index < screen->items_count )
        strcpy( scroll_item, screen->items[ scroll_index ].genre_string );

    int genres_count = 0;
    int total_album_count = 0;
    musicdb_genre_t* genres = musicdb_genres_get( musicdb, &genres_count, &total_album_count );

    if( remap_genre )
        {
        for( int i = 1; i < genres_count; ++i )
            {
            for( int j = 0; j < i; ++j )
                {
                if( stricmp( genres[ i ].remapped_label, genres[ j ].remapped_label ) == 0 )
                    {
                    genres[ j ].album_count += genres[ i ].album_count;
                    genres[ i ] = genres[ --genres_count ];
                    --i;
                    break;
                    }
                }            
            }
        }


    if( remap_genre )
        qsort( (void*)genres, genres_count, sizeof( *genres), sort_genres_remapped );
    else
        qsort( (void*)genres, genres_count, sizeof( *genres), sort_genres );

    screen_genres_item_t* new_items = (screen_genres_item_t*) malloc( sizeof( *new_items ) * ( genres_count + 1 ) );
    int new_items_count = 0;

    char str[ 256 ];
    new_items[ new_items_count++ ] = screen->items[ 0 ];
    memset( &screen->items[ 0 ], 0, sizeof( screen_genres_item_t ) );
    screen->items[ 0 ].genre_id = MUSICDB_INVALID_ID;

    screen_genres_item_t* all_artists = &new_items[ 0 ];
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
                screen_genres_item_t* item = &new_items[ new_items_count++ ];
                *item = screen->items[ j ];
                memset( &screen->items[ j ], 0, sizeof( screen_genres_item_t ) );
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
            screen_genres_item_t* item = &new_items[ new_items_count++ ];
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
        screen_genres_item_t* item = &screen->items[ i ];
        if( item->bitmap && item->bitmap != screen->allartists ) bitmap_destroy( screen->render, item->bitmap );
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
            screen_genres_item_t* item = &screen->items[ i ];
            if( stricmp( item->genre_string, scroll_item ) == 0 )
                scroll_index = i;
            }
        }
    screen->scroll = scroll_offset + 3 * ( scroll_index / screen->layout_hcount );

    musicdb_genres_release( musicdb, genres );
    screen->force_resize = true;
    screen_genres_update_thumbs( screen, musicdb );
    }


typedef struct screen_genres_navigation_t
    {
    screen_t screen;
    uint32_t selected_genre_id;
    uint32_t genre_thumb_id;
    } screen_genres_navigation_t;


bool screen_genres_update( screen_genres_t* screen, screen_genres_navigation_t* navigation, bool resize, input_t const* input )
    {
    bool redraw = false;
    navigation->screen = SCREEN_GENRES;
    navigation->selected_genre_id = MUSICDB_INVALID_ID;
    navigation->genre_thumb_id = MUSICDB_INVALID_ID;

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
        int screen_steps = ( (int)screen->render->logical_height - 100 ) / ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
        int scroll_steps = ( 3 * scroll_end ) / ( screen->layout_size + screen->layout_vspacing );
        screen->layout_max_scroll = scroll_steps - screen_steps;
        if( screen->layout_max_scroll < 0 ) screen->layout_max_scroll = 0;
        screen->scroll = scroll_offset + 3 * ( scroll_item / screen->layout_hcount );
        if( screen->scroll > screen->layout_max_scroll ) screen->scroll = screen->layout_max_scroll;

        for( int i = 0; i < screen->items_count; ++i )
            {
            screen_genres_item_t* item = &screen->items[ i ];
            if( item->genre_text ) text_destroy( screen->render, item->genre_text );
            item->genre_text = 0;
            if( item->album_count_text ) text_destroy( screen->render, item->album_count_text );
            item->album_count_text = 0;
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
    int x = 0;
    int y = -screen->scroll * ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_genres_item_t* item = &screen->items[ i ];
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

    if( highlighted != screen->highlighted )
        {
        screen->highlighted = highlighted;
        redraw = true;
        }

    return redraw;
    }


bool screen_genres_draw( screen_genres_t* screen )
    {
    render_scissor( screen->render, 0, 80, (int)screen->render->logical_width, (int)screen->render->logical_height );
    int x = 0;
    int y = -screen->scroll * ( ( screen->layout_size + screen->layout_vspacing ) / 3 );
    for( int i = 0; i < screen->items_count; ++i )
        {
        screen_genres_item_t* item = &screen->items[ i ];
        if( !item->genre_text ) 
            item->genre_text = text_create( screen->render, screen->font_genre, item->genre_string );
        if( !item->album_count_text ) 
            item->album_count_text = text_create( screen->render, screen->font_albums, item->album_count_string );
        
        bitmap_draw( screen->render, item->bitmap ? item->bitmap : screen->genredefault, 25 + x + screen->layout_offset, y + 100, screen->layout_size, screen->layout_size, 0xffffffff );
        uint32_t col = 0xffffffff;
        if( screen->highlighted == i )
            {
            col = 0xffd98b1a;
            bitmap_draw( screen->render, screen->frame, screen->layout_hspacing + x + screen->layout_offset - 8, y + 100 - 8, 266, 266, col );
            }
        text_draw( screen->render, item->genre_text, screen->layout_hspacing + x + ( screen->layout_size - item->genre_text->width ) / 2 + screen->layout_offset, y + 100 + screen->layout_size + 36, col );
        text_draw( screen->render, item->album_count_text, screen->layout_hspacing + x + ( screen->layout_size - item->album_count_text->width ) / 2 + screen->layout_offset, y + 100 + screen->layout_size + 60, 0xff736d64 );
        x += screen->layout_size + screen->layout_hspacing * 2;
        if( x >= screen->layout_hcount * ( screen->layout_size + screen->layout_hspacing * 2 ) )
            {
            x = 0;
            y += screen->layout_size + screen->layout_vspacing;
            }

        }
    render_scissor_off( screen->render );
    return false;
    }
