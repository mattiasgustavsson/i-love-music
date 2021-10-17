
typedef struct screen_settings_t
    {
    music_db_t* musicdb;
    app_t* app;
    render_t* render;
    int width;
    int height;

    font_t* font;

    } screen_settings_t;


bool screen_settings_init( screen_settings_t* screen, app_t* app, render_t* render, music_db_t* musicdb )
    {
    memset( screen, 0, sizeof( *screen ) );
    screen->app = app;
    screen->render = render;
    screen->musicdb = musicdb;

    screen->width = (int)render->logical_width;
    screen->height = (int)render->logical_height;

    if( !( screen->font = font_create( screen->render, "gui/fonts/AlegreyaSans-Medium.ttf", 24 ) ) ) return false;

    file_t* ini_file = file_load( "settings.ini", FILE_MODE_TEXT, NULL );
    if( ini_file ) {
        ini_t* ini = ini_load( ini_file->data, NULL );
        file_destroy( ini_file );
        if( ini ) {
            int property_music_path = ini_find_property( ini, INI_GLOBAL_SECTION, "music_path", 0 );
            if( property_music_path != INI_NOT_FOUND )
                {
                strcpy( global_music_path, ini_property_value( ini, INI_GLOBAL_SECTION, property_music_path ) );
                }
            int property_scale = ini_find_property( ini, INI_GLOBAL_SECTION, "scale", 0 );
            if( property_scale != INI_NOT_FOUND )
                {
                global_scale = (float) atof( ini_property_value( ini, INI_GLOBAL_SECTION, property_scale ) );
                }
            int property_touch_input = ini_find_property( ini, INI_GLOBAL_SECTION, "touch_input", 0 );
            if( property_touch_input!= INI_NOT_FOUND )
                {
                global_touch_input = stricmp( ini_property_value( ini, INI_GLOBAL_SECTION, property_touch_input ), "true" ) == 0;
                }
            int property_pause_on_lock = ini_find_property( ini, INI_GLOBAL_SECTION, "pause_on_lock", 0 );
            if( property_pause_on_lock!= INI_NOT_FOUND )
                {
                global_pause_on_lock = stricmp( ini_property_value( ini, INI_GLOBAL_SECTION, property_pause_on_lock ), "true" ) == 0;
                }
            int property_ffwd_speed = ini_find_property( ini, INI_GLOBAL_SECTION, "ffwd_speed", 0 );
            if( property_ffwd_speed != INI_NOT_FOUND )
                {
                g_ffwd_speed = (int) atoi( ini_property_value( ini, INI_GLOBAL_SECTION, property_ffwd_speed ) );
                }
            int property_rewind_speed = ini_find_property( ini, INI_GLOBAL_SECTION, "rewind_speed", 0 );
            if( property_rewind_speed != INI_NOT_FOUND )
                {
                g_rewind_speed = (int) atoi( ini_property_value( ini, INI_GLOBAL_SECTION, property_rewind_speed ) );
                }
            ini_destroy( ini );
        }
    }

    return true;
    }


void screen_settings_term( screen_settings_t* screen )
    {
    font_destroy( screen->render, screen->font );
    }


typedef struct screen_settings_navigation_t
    {
    screen_t screen;
    } screen_settings_navigation_t;


bool screen_settings_update( screen_settings_t* screen, screen_settings_navigation_t* navigation, bool resize, input_t const* input )
    {
    (void) input;
    bool redraw = false;
    navigation->screen = SCREEN_SETTINGS;

    if( resize || screen->width != (int)screen->render->logical_width || screen->height != (int)screen->render->logical_height )
        {

        screen->width = (int)screen->render->logical_width;
        screen->height = (int)screen->render->logical_height;

        redraw = true;
        }

    return redraw;
    }


bool screen_settings_draw( screen_settings_t* screen )
    {
    render_scissor( screen->render, 0, 80, (int)screen->render->logical_width, (int)screen->render->logical_height );

    render_scissor_off( screen->render );
    return false;
    }
