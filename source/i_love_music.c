#define _CRTDBG_MAP_ALLOC
#define _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_SECURE_NO_WARNINGS

#ifndef UNICODE
    #define UNICODE
#endif


#ifndef NDEBUG
	#pragma warning( push ) 
	#pragma warning( disable: 4619 ) // pragma warning : there is no warning number 'number'
	#pragma warning( disable: 4668 ) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
	#include <crtdbg.h>
	#pragma warning( pop ) 
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <float.h>
#include <stdint.h>
#include <sys/stat.h>

float global_scale = 1.0f;
bool global_touch_input = false;
bool global_pause_on_lock = false;
bool g_noscan = false;
bool g_fullscreen = false;
char global_music_path[512] = {0};
bool g_init_done = false;
bool g_splash_screen_showing = false;
float g_global_volume = 1.0f;
int g_ffwd_speed = 6;
int g_rewind_speed = 6;
bool g_mute = false;
void auto_levels( uint32_t* pixels, int w, int h );
void auto_contrast( uint32_t* pixels, int w, int h );


#include "libs/app.h"
#include "libs/dir.h"
#include "libs/file.h"
#include "libs/file_util.h"
#include "libs/filewatch.h"
#include "libs/frametimer.h"
#include "libs/glad.h"
#include "libs/id3tag.h"
#include "libs/ini.h"
#include "libs/mmap.h"
#include "libs/trirast.h"




struct 
    {
    char const* original;
    char const* remapped;
    } genres_mappings[4096];



void load_genre_mappings( void ) {
    file_t* ini_file = file_load( "genre_remap.ini", FILE_MODE_TEXT, NULL );
    if( ini_file ) {
        ini_t* ini = ini_load( ini_file->data, NULL );
        file_destroy( ini_file );
        if( ini ) {
            for( int i = 0; i < ini_property_count( ini, INI_GLOBAL_SECTION ); ++i )
                {
                if( i >= sizeof( genres_mappings ) / sizeof( *genres_mappings ) ) break;
                genres_mappings[ i ].original = strdup( ini_property_name( ini, INI_GLOBAL_SECTION, i ) );
                genres_mappings[ i ].remapped = strdup( ini_property_value( ini, INI_GLOBAL_SECTION, i ) );
                }
            ini_destroy( ini );
        }
    }
}


#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#define MA_NO_WAV
#define MA_NO_MP3
#define MA_NO_FLAC
#pragma warning( push )
#pragma warning( disable: 4255 )
#pragma warning( disable: 4668 )
#include "libs/miniaudio.h"
#pragma warning( pop )

#include "libs/minimp3_ex.h"
#include "libs/rnd.h"
#include "libs/stb_image.h"
#include "libs/stb_image_resize.h"
#include "libs/stb_truetype.h"
#include "libs/thread.h"
#include "libs/vecmath.h"

#pragma warning( disable: 4204 )

uint64_t calc_hash( void const* data, size_t size );
void copy_to_clipboard( char const* str );
int copy_from_clipboard( char* str, size_t capacity );
void tray_icon( app_t* app );
void tray_tooltip( app_t* app, char const* tooltip );
void tray_icon_off( app_t* app );
void toggle_screenmode( app_t* app );
bool computer_locked( void );
void hide_window( app_t* app );
void show_window( app_t* app );
bool lbutton_down( void );
bool bring_up_from_minimized( app_t* app );
bool is_maximized( app_t* app );
void maximize( app_t* app, bool zoom );


void sleep( int ms )
    {
    uint64_t ns = 1000000ULL * (uint64_t)ms;
    thread_timer_t timer;
    thread_timer_init( &timer );
    thread_timer_wait( &timer, ns );
    thread_timer_term( &timer );
    }

int random( int min, int max ) {
    static bool initialized = false;
    static rnd_pcg_t pcg;
    if( !initialized ) {
        rnd_pcg_seed( &pcg, (RND_U32) time( NULL) );
        initialized = true;
    }
    return rnd_pcg_range( &pcg, min, max );
}


void shuffle_ints( int* elements, int count )
	{
	for( int i = 0; i < count; ++i )
		{
		int r = random( i, count - 1 );
        int t = elements[ r ];
        elements[ r ] = elements[ i ];
        elements[ i ] = t;
		}
	}

unsigned int pow2_ceil( unsigned int x );

typedef struct img_t { uint32_t* pixels; int width; int height; } img_t;

img_t* img_create( char const* filename );
void img_destroy( img_t* img );
void img_resize( img_t* img, int width, int height );


int global_max_w = 256;
int global_max_h = 256;

struct render_t;
struct bitmap_t* load_app_bitmap( struct render_t* render, char const* source_file, char const* cached_file, int max_w, int max_h );
uint32_t* make_mipmap_chain( char const* filename, int* width, int* height, int max_w, int max_h );
void free_mipmap_chain( uint32_t* mipmaps );

img_t* g_shadow; // TODO: Don't use a global for this

void change_current_task( char const* str )
    {
    static char const* current = "";
    if( str != current )
        {
        current = str;
        printf( "%s\n", current );
        }
    }

bool g_shuffle_list_loaded = false;

#pragma warning( disable: 4702 )
#pragma warning( disable: 4706 )

#include "artist_thumbnail_generator.h"
#include "background_tasks.h"
#include "render.h"
#include "musicdb.h"
#include "play_thread.h"
#include "play_control.h"

typedef struct input_t
    {
    int mouse_x;
    int mouse_y;
    int scroll;
    bool click;
    int click_start_x;
    int click_start_y;
    bool dblclick;
    bool pgup;
    bool pgdown;
    bool up;
    bool down;
    bool left;
    bool right;
    bool home;
    bool end;
    bool space;
    bool ctrl;
    bool shift;
    bool lbutton;
    bool key_play_pause;
    bool key_next_track;
    bool key_prev_track;
    bool zero;
    bool question;
    bool letters_a_to_z[ 26 ];    
} input_t;

uint32_t const text_col_active = 0xffffffff;
uint32_t const text_col_highlighted = 0xffd98b1a;
uint32_t const text_col = 0xff736d64;

typedef enum screen_t
    {
    SCREEN_GENRES,
    SCREEN_ARTISTS,
    SCREEN_ALBUMS,
    SCREEN_TRACKS,
    SCREEN_SHUFFLE,
    SCREEN_SETTINGS,
    } screen_t;

#include "screen_genres.h"
#include "screen_artists.h"
#include "screen_albums.h"
#include "screen_tracks.h"
#include "screen_shuffle.h"
#include "screen_settings.h"

#include "tabs_bar.h"

static void audio_data_callback( ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount )
    {
    (void) pInput;
    sound_proc( (short*) pOutput, (int) frameCount, pDevice->pUserData );
    }


typedef struct last_state_t {
    bool fullscreen;
    bool maximized;
    int window_x;
    int window_y;
    int window_width;
    int window_height;
    int tab;
    float volume;
    bool repeat;
    bool loop;
    bool shuffle;
    uint32_t album;
    int track;
    uint32_t tracks_album;
    int genres_scroll;
    int artists_scroll;
    int albums_scroll;
    uint32_t artists_genre_id;
    uint32_t artists_genre_thumb;
    uint32_t albums_genre_id;
    uint32_t albums_artist_id;
    int shuffle_list_index;
} last_state_t;


void load_last_state( last_state_t* state ) {
    state->fullscreen = false;
    state->maximized = false;
    state->window_x = 0;
    state->window_y = 0;
    state->window_width = 0;
    state->window_height = 0;
    state->tab = 0;
    state->volume = 1.0f;
    state->repeat = true;
    state->loop = false;
    state->shuffle = false;
    state->album = MUSICDB_INVALID_ID;
    state->track = -1;
    state->tracks_album = MUSICDB_INVALID_ID;
    state->genres_scroll = 0;
    state->artists_scroll = 0;
    state->albums_scroll = 0;
    state->artists_genre_id = MUSICDB_INVALID_ID;
    state->artists_genre_thumb = MUSICDB_INVALID_ID;
    state->albums_genre_id = MUSICDB_INVALID_ID;
    state->albums_artist_id = MUSICDB_INVALID_ID;
    state->shuffle_list_index = 0;
    file_t* ini_file = file_load( ".cache\\last_state.ini", FILE_MODE_TEXT, NULL );
    if( ini_file ) {
        ini_t* ini = ini_load( ini_file->data, NULL );
        file_destroy( ini_file );
        if( ini ) {
            char const* fullscreen = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "fullscreen", 0 ) );
            state->fullscreen = fullscreen && atoi( fullscreen ) != 0;

            char const* maximized = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "maximized", 0 ) );
            state->maximized = maximized && atoi( maximized ) != 0;

            char const* window_x = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "window_x", 0 ) );

            char const* window_y = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "window_y", 0 ) );

            char const* window_width = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "window_width", 0 ) );

            char const* window_height = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "window_height", 0 ) );

            if( window_x && window_y && window_width && window_height )
                {
                state->window_x = (int)atoi( window_x );
                state->window_y = (int)atoi( window_y );
                state->window_width = (int)atoi( window_width );
                state->window_height = (int)atoi( window_height );
                }

            char const* tab = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "tab", 0 ) );
            state->tab = tab ? atoi( tab ) : state->tab;

            char const* volume = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "volume", 0 ) );
            state->volume = volume ? (float)atof( volume ) : state->volume;

            char const* repeat = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "repeat", 0 ) );
            state->repeat = repeat && atoi( repeat ) != 0;

            char const* loop = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "loop", 0 ) );
            state->loop = loop && atoi( loop ) != 0;

            char const* shuffle = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "shuffle", 0 ) );
            state->shuffle = shuffle && atoi( shuffle ) != 0;

            char const* album = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "album", 0 ) );
            state->album = album ? (uint32_t)(int)atoll( album ) : state->album;

            char const* track = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "track", 0 ) );
            state->track = track ? (int)atoi( track ) : state->track;

            char const* tracks_album = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "tracks_album", 0 ) );
            state->tracks_album = tracks_album ? (uint32_t)(int)atoll( tracks_album ) : state->tracks_album;

            char const* genres_scroll = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "genres_scroll", 0 ) );
            state->genres_scroll = genres_scroll ? (int)atoi( genres_scroll ) : state->genres_scroll;

            char const* artists_scroll = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "artists_scroll", 0 ) );
            state->artists_scroll = artists_scroll ? (int)atoi( artists_scroll ) : state->artists_scroll;

            char const* albums_scroll = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "albums_scroll", 0 ) );
            state->albums_scroll = albums_scroll ? (int)atoi( albums_scroll ) : state->albums_scroll;

            char const* artists_genre_id = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "artists_genre_id", 0 ) );
            state->artists_genre_id = artists_genre_id ? (uint32_t)(int)atoll( artists_genre_id ) : state->artists_genre_id;

            char const* artists_genre_thumb = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "artists_genre_thumb", 0 ) );
            state->artists_genre_thumb = artists_genre_thumb ? (uint32_t)(int)atoll( artists_genre_thumb ) : state->artists_genre_thumb;

            char const* albums_genre_id = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "albums_genre_id", 0 ) );
            state->albums_genre_id = albums_genre_id ? (uint32_t)(int)atoll( albums_genre_id ) : state->albums_genre_id;

            char const* albums_artist_id = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "albums_artist_id", 0 ) );
            state->albums_artist_id = albums_artist_id ? (uint32_t)(int)atoll( albums_artist_id ) : state->albums_artist_id;

            char const* shuffle_list_index = ini_property_value( ini, INI_GLOBAL_SECTION, 
                ini_find_property( ini, INI_GLOBAL_SECTION, "shuffle_list_index", 0 ) );
            state->shuffle_list_index = shuffle_list_index ? (int)atoi( shuffle_list_index ) : state->shuffle_list_index;

            ini_destroy( ini );
        }
    }
}


int app_proc( app_t* app, void* user_data )
    {
    (void) user_data;
    printf( "start\n" );
    app_title_unicode( app, L"I \u2764 Music" );

    load_genre_mappings();

    last_state_t last_state;
    load_last_state( &last_state );

    if( !( last_state.window_x == 0 && last_state.window_y == 0 && last_state.window_width == 0 && last_state.window_height == 0 ) ) 
        {
        app_screenmode( app, APP_SCREENMODE_WINDOW );
        app_window_pos( app, last_state.window_x, last_state.window_y );
        app_window_size( app, last_state.window_width, last_state.window_height );
        app_yield( app );
        }

    if( g_fullscreen || last_state.fullscreen )
        {
        app_screenmode( app, APP_SCREENMODE_FULLSCREEN );
        g_fullscreen = true;
        }
    else
        {
        app_screenmode( app, APP_SCREENMODE_WINDOW );
        g_fullscreen = false;
        if( last_state.maximized ) maximize( app, true );
        }

    tray_icon( app );

    g_shadow = img_create( "gui/shadow.png" );
    int shadow_h = global_max_h * 2;
    int shadow_w = ( g_shadow->width * shadow_h ) / g_shadow->height;
    img_resize( g_shadow, shadow_w, shadow_h );

    background_tasks_t background_tasks;
    background_tasks_init( &background_tasks );

    uint64_t db_change_counter = 0;

    bool remap_genres = true;

    printf( "music db\n" );
    bool rebuild_db = false;
    music_db_t musicdb;
    if( !musicdb_open( &musicdb, &background_tasks, ".cache", rebuild_db ) )
        {
        if( !musicdb_open( &musicdb, &background_tasks, ".cache", true ) )
            app_fatal_error( app, "Failed to create database" );
        }

    uint32_t tooltip_album_id = MUSICDB_INVALID_ID;
    int tooltip_track_index = -1;


    printf( "gl\n" );
    int version = gladLoaderLoadGL();
    printf("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));


    int height = 1024;
    int width = (int)( 1024.0f * ( (float) app_window_width( app ) / (float) app_window_height( app ) ) );

    printf( "render\n" );
    render_t render;
    if( !render_init( &render, app, width, height ) ) app_fatal_error( app, "Failed to initialize renderer" );

    slider_bitmaps_t slider_bitmaps;
    if( !( slider_bitmaps.left = load_app_bitmap( &render, "gui/slider_left.png", ".cache/gui/slider_left.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );
    if( !( slider_bitmaps.middle = load_app_bitmap( &render, "gui/slider_middle.png", ".cache/gui/slider_middle.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );
    if( !( slider_bitmaps.right = load_app_bitmap( &render, "gui/slider_right.png", ".cache/gui/slider_right.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );
    if( !( slider_bitmaps.gap = load_app_bitmap( &render, "gui/slider_gap.png", ".cache/gui/slider_gap.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );
    if( !( slider_bitmaps.handle = load_app_bitmap( &render, "gui/slider_handle.png", ".cache/gui/slider_handle.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );

    printf( "preload bitmaps\n" );
    bitmap_t* albumdefault;
    if( !( albumdefault = load_app_bitmap( &render, "gui/albumdefault.png", ".cache/gui/albumdefault.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );

    bitmap_t* allalbums;
    if( !( allalbums = load_app_bitmap( &render, "gui/allalbums.png", ".cache/gui/allalbums.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );

    bitmap_t* allartists;
    if( !( allartists = load_app_bitmap( &render, "gui/allartists.png", ".cache/gui/allartists.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );

    bitmap_t* background;
    if( !( background = load_app_bitmap( &render, "gui/background.png", ".cache/gui/background.mip", 512, 512 ) ) ) app_fatal_error( app, "Failed to load texture" );

    bitmap_t* frame;
    if( !( frame = load_app_bitmap( &render, "gui/frame.png", ".cache/gui/frame.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );

    bitmap_t* genredefault;
    if( !( genredefault = load_app_bitmap( &render, "gui/genredefault.png", ".cache/gui/genredefault.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );

    bitmap_t* variousartists;
    if( !( variousartists = load_app_bitmap( &render, "gui/variousartists.png", ".cache/gui/variousartists.mip", global_max_w, global_max_h ) ) ) app_fatal_error( app, "Failed to load texture" );

    printf( "screen settings\n" );
    screen_settings_t screen_settings;
    screen_settings_init( &screen_settings, app, &render, &musicdb );

    printf( "play thread\n" );
    play_thread_t* play = play_thread_create();
    static play_control_t control;
    play_control_init( &control, play );
    control.repeat = last_state.repeat;
    control.shuffle = last_state.shuffle;
    control.loop = last_state.loop;
    if( last_state.album != MUSICDB_INVALID_ID ) 
        {
        int songs_count = 0;
        musicdb_song_t* songs = musicdb_songs_get( &musicdb, last_state.album, &songs_count, NULL, NULL );   
        if( songs && songs_count )
            {
            control.play_thread->ignore_resume = true;
            play_control_pause( &control );
            play_control_album( &control, last_state.album, songs, songs_count );
            if( last_state.track >= 0 && last_state.track < songs_count )
                play_control_track( &control, last_state.track );
            play_control_pause( &control );
            musicdb_songs_release( &musicdb, songs );
            }
        }


    printf( "tabs bar\n" );
    tabs_bar_t* tabs_bar = tabs_bar_create( &render, &control, slider_bitmaps );
    if( last_state.tab >= 0 && last_state.tab <= 4 )
        tabs_bar->active_tab_index = last_state.tab + 1;
    else
        tabs_bar->active_tab_index = 1;

    printf( "screen genres\n" );
    screen_genres_t screen_genres;
    screen_genres_init( &screen_genres, app, &render, genredefault, allartists, frame );
    screen_genres_refresh( &screen_genres, &musicdb, remap_genres );
    bool load_genre_thumbs = true;

    printf( "screen artists\n" );
    screen_artists_t screen_artists;
    screen_artists_init( &screen_artists, app, &render, genredefault, allalbums, variousartists, frame );
    if( last_state.artists_genre_id != MUSICDB_INVALID_ID && last_state.artists_genre_thumb != MUSICDB_INVALID_ID ) 
        {
        screen_artists_set_genre( &screen_artists, last_state.artists_genre_id, last_state.artists_genre_thumb );
        }

    printf( "screen albums\n" );
    screen_albums_t screen_albums;
    screen_albums_init( &screen_albums, app, &render, albumdefault, frame );
    if( last_state.albums_artist_id != MUSICDB_INVALID_ID && last_state.albums_genre_id != MUSICDB_INVALID_ID ) 
        {
        screen_albums_set_various_artists( &screen_albums, last_state.albums_artist_id, last_state.albums_genre_id );
        }
    else if( last_state.albums_artist_id != MUSICDB_INVALID_ID ) 
        {
        screen_albums_set_artist( &screen_albums, last_state.albums_artist_id );
        }
    else if( last_state.albums_genre_id != MUSICDB_INVALID_ID ) 
        {
        screen_albums_set_genre( &screen_albums, last_state.albums_genre_id );
        }
    
    printf( "screen tracks\n" );
    screen_tracks_t screen_tracks;
    screen_tracks_init( &screen_tracks, app, &render, &control, frame, slider_bitmaps );

    printf( "screen shuffle\n" );
    screen_shuffle_t screen_shuffle;
    screen_shuffle_init( &screen_shuffle, app, &render );
    screen_shuffle.track_index = last_state.shuffle_list_index;

    printf( "almost done\n" );
    screen_t screen = last_state.tab >= 0 && last_state.tab <= SCREEN_SHUFFLE ? (screen_t) last_state.tab : SCREEN_GENRES;
    screen_t prev_screen = SCREEN_SETTINGS;

    if( g_noscan == false ) 
        {
        char const* paths[] = { ".", global_music_path };
        musicdb_build( &musicdb, paths, *global_music_path ? 2 : 1 );
        }

    musicdb_load_shuffle_list( &musicdb );
    
    time_t last_refresh = 0;
    time( &last_refresh );
    bool first_time_refresh = true;

    frametimer_t* frametimer = frametimer_create( 0 );
    frametimer_lock_rate( frametimer, 60 );

    int win_x = app_window_x( app );
    int win_y = app_window_y( app );
    int win_w = app_window_width( app );
    int win_h = app_window_height( app );

    bool redraw = true;
    int start_count = 2;
    int draw_disable = 0;

    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool lbutton = false;
    bool prev_lbutton = false;
    int scroll_accumulate = 0;
    int scroll_prev = 0;
    int click_start_x = 0;
    int click_start_y = 0;

    bool is_computer_locked = false;
    bool was_paused_before_locked = true;

    if( last_state.tracks_album != MUSICDB_INVALID_ID )
        screen_tracks_set_album( &screen_tracks, last_state.tracks_album );

    ma_device_config config = ma_device_config_init( ma_device_type_playback );
    config.playback.format   = ma_format_s16;
    config.playback.channels = 2;
    config.sampleRate        = 44100;
    config.dataCallback      = audio_data_callback;
    config.pUserData         = play;

    ma_device device;
    if ( ma_device_init( NULL, &config, &device ) != MA_SUCCESS ) 
        {
        //return -1;  // Failed to initialize the device.
        }

    ma_device_start( &device );

    printf( "init done\n" );
    bool first_frame = true;
    bool applied_last_state_scroll = false;
    bool minimized = false;
    g_init_done = true;
    while( g_splash_screen_showing ) {
        sleep( 100 );
    }
    g_global_volume = last_state.volume;
    float prev_volume = g_global_volume;
    slider_value_set( &tabs_bar->volume, g_global_volume );
    bool prev_mute = g_mute;
    while( app_yield( app ) != APP_STATE_EXIT_REQUESTED )
        {
        if( g_global_volume != prev_volume ) {
            prev_volume = g_global_volume;
            ma_device_set_master_volume( &device, g_global_volume );
        }
        if( g_mute && !prev_mute )
            ma_device_set_master_volume( &device, 0.0f );
        else if( !g_mute && prev_mute )
            ma_device_set_master_volume( &device, g_global_volume );
        prev_mute = g_mute;

        bool lbutton_released = !lbutton_down();

        if( control.album_id != tooltip_album_id || play_control_track_index( &control ) != tooltip_track_index )
        {
            char tooltip[ 256 ] = "";
            if( play_control_track_index( &control ) >= 0 && play_control_track_index( &control ) < control.tracks_count )
            {
                sprintf( tooltip, "\n%s\n%s", control.tracks[ play_control_track_index( &control ) ].artist, control.tracks[ play_control_track_index( &control ) ].title );
                tray_tooltip( app, tooltip );
            }
            tooltip_album_id = control.album_id;
            tooltip_track_index = play_control_track_index( &control );
        }
        frametimer_update( frametimer );
        time_t now = 0;
        time( &now );

        if( !render_view_valid( &render ) ) 
            {
            minimized = true;
            redraw = true;
            if( !bring_up_from_minimized( app ) )
                continue;
            }

        if( minimized ) { minimized = false; redraw = true; }

        int new_win_x = app_window_x( app );
        int new_win_y = app_window_y( app );
        int new_win_w = app_window_width( app );
        int new_win_h = app_window_height( app );

        bool resize = start_count > 0;
        if( start_count > 0 ) --start_count;
        if( win_x != new_win_x || win_y != new_win_y )
            {
            win_x = new_win_x;
            win_y = new_win_y;
            redraw = true;
            }

        if( win_w != new_win_w || win_h != new_win_h )
            {
            win_w = new_win_w;
            win_h = new_win_h;
            redraw = true;
            float scale = 1.0f;
            if( render.view_height < 300.0f ) scale = render.view_height / 300.0f;
            height = (int)( 1024 * scale );
            width = (int)( 1024.0f * scale * ( render.view_width / render.view_height ) );
            if( width < height )
                {
                if( render.view_width < 400.0f ) scale = render.view_width / 400.0f;
                width = (int)( 1280 * scale );
                height = (int)( 1280.0f * scale * ( render.view_height / render.view_width ) );
                }         
            }


        if( render_update_window( &render, (int)( width / global_scale ), (int)( height / global_scale ) ) )
            {
            redraw = true;
            resize = true;
            }

        if( resize )
            {
            float scale = 1.0f;
            if( render.view_height < 300.0f ) scale = render.view_height / 300.0f;
            height = (int)( 1024 * scale );
            width = (int)( 1024.0f * scale * ( render.view_width / render.view_height ) );
            if( width < height )
                {
                if( render.view_width < 400.0f ) scale = render.view_width / 400.0f;
                width = (int)( 1280 * scale );
                height = (int)( 1280.0f * scale * ( render.view_height / render.view_width ) );
                }         
            }

        if( global_pause_on_lock ) 
            {
            if( is_computer_locked && !computer_locked() )
                {
                printf( "Unlock\n" );
                is_computer_locked = false;
                if( !was_paused_before_locked )
                    play_control_play( &control );
                }
            else if( !is_computer_locked && computer_locked() )
                {
                printf( "Lock\n" );
                is_computer_locked = true;
                was_paused_before_locked = control.is_paused;
                play_control_pause( &control );
                }
            }

        input_t input = { 0 };
        input.mouse_x = app_pointer_x( app );
        input.mouse_y = app_pointer_y( app );
        input.mouse_x = (int)( ( input.mouse_x / render.view_width ) * render.logical_width );
        input.mouse_y = (int)( ( input.mouse_y / render.view_height ) * render.logical_height );
        input.scroll = 0;
        input.click = false;
        input.dblclick = false;
        input.pgup = false;
        input.pgdown = false;
        input.up = false;
        input.down = false;
        input.left = false;
        input.right = false;
        input.home = false;
        input.end = false;
        input.space = false;
        input.key_play_pause = false;
        input.key_prev_track = false;
        input.key_next_track = false;
        input.click_start_x = click_start_x;
        input.click_start_y = click_start_y;
        
        app_input_t appinput = app_input( app );
        for( int i = 0; i < appinput.count; ++i )
            {
            if( global_touch_input ) 
                {
                if( appinput.events[ i ].type == APP_INPUT_MOUSE_MOVE )
                    {
                    if( lbutton ) 
                        {
                        scroll_accumulate += input.mouse_y - scroll_prev;
                        scroll_prev = input.mouse_y;
                        }
                    }
                }
            if( appinput.events[ i ].type == APP_INPUT_CHAR ) 
                {
                int code = (int)appinput.events[ i ].data.char_code;
                if( code >= 'A' && code <= 'Z' )
                    {
                    input.letters_a_to_z[ code - 'A' ] = true;
                    }
                if( code >= 'a' && code <= 'z' )
                    {
                    input.letters_a_to_z[ code - 'a' ] = true;
                    }
                if( code == '0' ) 
                    {
                    input.zero = true;
                    }
                if( code == '?' ) 
                    {
                    input.question = true;
                    }
                }
            if( appinput.events[ i ].type == APP_INPUT_SCROLL_WHEEL )
                {
                if( appinput.events[ i ].data.wheel_delta < 0 ) ++input.scroll;
                if( appinput.events[ i ].data.wheel_delta > 0 ) --input.scroll;
                }
            if( appinput.events[ i ].type == APP_INPUT_DOUBLE_CLICK )
                {
                input.dblclick = true;
                }
            if( appinput.events[ i ].type == APP_INPUT_KEY_UP )
                {
                if( appinput.events[ i ].data.key == APP_KEY_MENU ) alt = false;
                if( appinput.events[ i ].data.key == APP_KEY_CONTROL ) ctrl = false;
                if( appinput.events[ i ].data.key == APP_KEY_SHIFT ) shift = false;
                if( appinput.events[ i ].data.key == APP_KEY_LBUTTON ) 
                    {
                    if( lbutton )
                        {
                        input.click_start_x = click_start_x;
                        input.click_start_y = click_start_y;
                        input.click = true;
                        }
                    lbutton = false;
                    }
                }
            if( appinput.events[ i ].type == APP_INPUT_KEY_DOWN )
                {
                if( appinput.events[ i ].data.key == APP_KEY_MENU ) alt = true;
                if( appinput.events[ i ].data.key == APP_KEY_CONTROL ) ctrl = true;
                if( appinput.events[ i ].data.key == APP_KEY_SHIFT ) shift = true;
                if( appinput.events[ i ].data.key == APP_KEY_PRIOR ) input.pgup = true;
                if( appinput.events[ i ].data.key == APP_KEY_NEXT ) input.pgdown = true;
                if( appinput.events[ i ].data.key == APP_KEY_MEDIA_PLAY_PAUSE ) input.key_play_pause = true;
                if( appinput.events[ i ].data.key == APP_KEY_MEDIA_NEXT_TRACK ) input.key_next_track = true;
                if( appinput.events[ i ].data.key == APP_KEY_MEDIA_PREV_TRACK ) input.key_prev_track = true;
                if( appinput.events[ i ].data.key == APP_KEY_UP ) input.up = true;
                if( appinput.events[ i ].data.key == APP_KEY_DOWN ) input.down = true;
                if( appinput.events[ i ].data.key == APP_KEY_LEFT ) input.left = true;
                if( appinput.events[ i ].data.key == APP_KEY_RIGHT ) input.right = true;
                if( appinput.events[ i ].data.key == APP_KEY_HOME ) input.home = true;
                if( appinput.events[ i ].data.key == APP_KEY_END ) input.end = true;
                if( appinput.events[ i ].data.key == APP_KEY_SPACE ) input.space = true;
                                
                if( appinput.events[ i ].data.key == APP_KEY_ESCAPE )
                    {
                    hide_window( app );
                    lbutton = false;
                    }
                if( appinput.events[ i ].data.key == APP_KEY_LBUTTON )
                    {
                    click_start_x = input.mouse_x;
                    click_start_y = input.mouse_y;
                    lbutton = true;
                    scroll_accumulate = 0;
                    scroll_prev = input.mouse_y;
                    }

                screen_t new_screen = screen;
                if( appinput.events[ i ].data.key == APP_KEY_TAB )
                    {
                    if( shift ) 
                        {
                        new_screen = screen - 1;
                        if( new_screen < 0 ) 
                            {
                            new_screen = SCREEN_SHUFFLE;
                            }
                        }
                    else 
                        {
                        new_screen = screen + 1;
                        if( new_screen > SCREEN_SHUFFLE ) 
                            {
                            new_screen = SCREEN_GENRES;
                            }
                        }
                    }

                if( appinput.events[ i ].data.key == APP_KEY_F1 )
                    {
                    new_screen = SCREEN_GENRES;
                    }
                if( appinput.events[ i ].data.key == APP_KEY_F2 )
                    {
                    new_screen = SCREEN_ARTISTS;
                    }
                if( appinput.events[ i ].data.key == APP_KEY_F3 )
                    {
                    new_screen = SCREEN_ALBUMS;
                    }
                if( appinput.events[ i ].data.key == APP_KEY_F4 )
                    {
                    new_screen = SCREEN_TRACKS;
                    }
                if( appinput.events[ i ].data.key == APP_KEY_F5 )
                    {
                    new_screen = SCREEN_SHUFFLE;
                    }

                if( screen != new_screen )
                    {
                    if( new_screen == SCREEN_GENRES )
                        {
                        tabs_bar->active_tab_index = 1;
                        screen = SCREEN_GENRES;
                        screen_genres_refresh( &screen_genres, &musicdb, remap_genres );
                        last_refresh = now;
                        redraw = true;
                        resize = true;
                        }
                    else if( new_screen == SCREEN_ARTISTS )
                        {
                        tabs_bar->active_tab_index = 2;
                        screen = SCREEN_ARTISTS;
                        screen_artists_refresh( &screen_artists, &musicdb, remap_genres );
                        last_refresh = now;
                        redraw = true;
                        resize = true;
                        }
                    else if( new_screen == SCREEN_ALBUMS )
                        {
                        tabs_bar->active_tab_index = 3;
                        screen = SCREEN_ALBUMS;
                        screen_albums_refresh( &screen_albums, &musicdb, remap_genres );
                        last_refresh = now;
                        redraw = true;
                        resize = true;
                        }
                    else if( new_screen == SCREEN_TRACKS )
                        {
                        tabs_bar->active_tab_index = 4;
                        screen = SCREEN_TRACKS;
                        screen_tracks_refresh( &screen_tracks, &musicdb );
                        last_refresh = now;
                        redraw = true;
                        resize = true;
                        }
                    else if( new_screen == SCREEN_SHUFFLE )
                        {
                        tabs_bar->active_tab_index = 5;
                        screen = SCREEN_SHUFFLE;
                        screen_shuffle_refresh( &screen_shuffle, &musicdb );
                        last_refresh = now;
                        redraw = true;
                        resize = true;
                        }
                    }
                if( appinput.events[ i ].data.key == APP_KEY_F11 )
                    {
                    toggle_screenmode( app );
                    redraw = true;
                    }
                if( appinput.events[ i ].data.key == APP_KEY_RETURN && alt )
                    {
                    toggle_screenmode( app );
                    redraw = true;
                    }
                }
            }
        
        if( g_shuffle_list_loaded ) 
            {
            g_shuffle_list_loaded = false;
            screen_shuffle_refresh( &screen_shuffle, &musicdb );
            }

        if( lbutton && lbutton_released ) 
            lbutton = false;

        prev_lbutton = lbutton;

        if( global_touch_input )
            {
            int const scroll_threshold = 50;
            if( abs( scroll_accumulate ) >= scroll_threshold ) 
                {
                click_start_x = -1000;
                click_start_y = -1000;
                int steps = scroll_accumulate / scroll_threshold;
                input.scroll -= steps;
                scroll_accumulate -= steps * scroll_threshold;
                }
            }

        input.ctrl = ctrl;
        input.shift = shift;
        input.lbutton = lbutton;

        redraw |= play_control_update( &control );

        tabs_bar_navigation_t tabs_nav;
        tabs_nav.screen = screen;
        redraw |= tabs_bar_update( tabs_bar, &tabs_nav, appinput, &input, resize, 
            screen == SCREEN_TRACKS ? screen_tracks.album_id : MUSICDB_INVALID_ID, 
            screen == SCREEN_ALBUMS ? screen_albums.current_artist_id : MUSICDB_INVALID_ID, 
            screen == SCREEN_ARTISTS ? screen_artists.current_genre_id : MUSICDB_INVALID_ID,
            screen == SCREEN_TRACKS && screen_tracks.big_pic, &musicdb );
        if( tabs_nav.hide ) 
            {
            hide_window( app );
            lbutton = false;
            }
        if( tabs_nav.screen != screen || tabs_nav.album_id != MUSICDB_INVALID_ID )
            {
            screen = tabs_nav.screen;
            if( screen == SCREEN_GENRES )
                {
                tabs_bar->active_tab_index = 1;
                screen_genres_refresh( &screen_genres, &musicdb, remap_genres );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            if( screen == SCREEN_ARTISTS )
                {
                tabs_bar->active_tab_index = 2;
                screen_artists_refresh( &screen_artists, &musicdb, remap_genres );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            if( screen == SCREEN_ALBUMS )
                {
                tabs_bar->active_tab_index = 3;
                screen = SCREEN_ALBUMS;
                screen_albums_refresh( &screen_albums, &musicdb, remap_genres );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            if( screen == SCREEN_TRACKS )
                {
                tabs_bar->active_tab_index = 4;
                if( tabs_nav.album_id != MUSICDB_INVALID_ID ) screen_tracks_set_album( &screen_tracks, tabs_nav.album_id );
                screen_tracks_refresh( &screen_tracks, &musicdb );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            if( screen == SCREEN_SHUFFLE )
                {
                screen_shuffle_refresh( &screen_shuffle, &musicdb );
                tabs_bar->active_tab_index = 5;
                redraw = true;
                resize = true;
                }
            if( screen == SCREEN_SETTINGS )
                {
                tabs_bar->active_tab_index = 0;
                redraw = true;
                }
            }

        if( screen == SCREEN_GENRES )
            {
            screen_genres_navigation_t navigation;
            redraw |= screen_genres_update( &screen_genres, &navigation, resize, &input );
            screen = navigation.screen;

            if( screen == SCREEN_ARTISTS )
                {
                tabs_bar->active_tab_index = 2;
                screen_artists_set_genre( &screen_artists, navigation.selected_genre_id, navigation.genre_thumb_id );
                screen_artists_refresh( &screen_artists, &musicdb, remap_genres );
                screen_albums_set_genre( &screen_albums, navigation.selected_genre_id );
                screen_albums_refresh( &screen_albums, &musicdb, remap_genres );
                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_albums_navigation_t nav;
                screen_albums_update( &screen_albums, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            }
        else if( screen == SCREEN_ARTISTS )
            {
            screen_artists_navigation_t navigation;
            redraw |= screen_artists_update( &screen_artists, &navigation, resize, &input );
            screen = navigation.screen;

            if( screen == SCREEN_ALBUMS )
                {
                tabs_bar->active_tab_index = 3;
                if( navigation.selected_artist_id == MUSICDB_INVALID_ID )
                    screen_albums_set_genre( &screen_albums, navigation.selected_genre_id );
                else if( navigation.selected_genre_id == MUSICDB_INVALID_ID )
                    screen_albums_set_artist( &screen_albums, navigation.selected_artist_id );
                else 
                    screen_albums_set_various_artists( &screen_albums, navigation.selected_artist_id, navigation.selected_genre_id );
                screen_albums_refresh( &screen_albums, &musicdb, remap_genres );
                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_albums_navigation_t nav;
                screen_albums_update( &screen_albums, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            else if( screen == SCREEN_TRACKS )
                {
                tabs_bar->active_tab_index = 4;
                screen_tracks_set_album( &screen_tracks, navigation.selected_album_id );
                screen_tracks_refresh( &screen_tracks, &musicdb );

                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_tracks_navigation_t nav;
                screen_tracks_update( &screen_tracks, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            else if( screen == SCREEN_ARTISTS && screen_artists.current_genre_id != navigation.selected_genre_id )
                {
                tabs_bar->active_tab_index = 2;
                screen_artists_set_genre( &screen_artists, navigation.selected_genre_id, navigation.genre_thumb_id );
                screen_artists_refresh( &screen_artists, &musicdb, remap_genres );
                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_artists_navigation_t nav;
                screen_artists_update( &screen_artists, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            }
        else if( screen == SCREEN_ALBUMS )
            {
            screen_albums_navigation_t navigation;
            redraw |= screen_albums_update( &screen_albums, &navigation, resize, &input );
            screen = navigation.screen;

            if( screen == SCREEN_TRACKS )
                {
                tabs_bar->active_tab_index = 4;
                screen_tracks_set_album( &screen_tracks, navigation.selected_album_id );
                screen_tracks_refresh( &screen_tracks, &musicdb );

                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_tracks_navigation_t nav;
                screen_tracks_update( &screen_tracks, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            else if( screen == SCREEN_ALBUMS && screen_albums.current_artist_id != navigation.selected_artist_id )
                {
                tabs_bar->active_tab_index = 3;
                screen_albums_set_artist( &screen_albums, navigation.selected_artist_id );
                screen_albums_refresh( &screen_albums, &musicdb, remap_genres );

                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_albums_navigation_t nav;
                screen_albums_update( &screen_albums, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            else if( screen == SCREEN_ALBUMS && screen_albums.current_genre_id != navigation.selected_genre_id )
                {
                tabs_bar->active_tab_index = 3;
                screen_albums_set_genre( &screen_albums, navigation.selected_genre_id );
                screen_albums_refresh( &screen_albums, &musicdb, remap_genres );

                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_albums_navigation_t nav;
                screen_albums_update( &screen_albums, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            }
        else if( screen == SCREEN_TRACKS )
            {
            screen_tracks_navigation_t navigation;
            redraw |= screen_tracks_update( &screen_tracks, &navigation, resize, &input );
            screen = navigation.screen;
            if( screen == SCREEN_ALBUMS )
                {
                tabs_bar->active_tab_index = 3;
                screen_albums_set_artist( &screen_albums, navigation.artist_id );
                screen_albums_refresh( &screen_albums, &musicdb, remap_genres );

                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_albums_navigation_t nav;
                screen_albums_update( &screen_albums, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            }
        else if( screen == SCREEN_SHUFFLE )
            {
            screen_shuffle_navigation_t navigation;
            redraw |= screen_shuffle_update( &screen_shuffle, &navigation, resize, &input, &musicdb, &control );
            screen = navigation.screen;
            if( screen == SCREEN_TRACKS )
                {
                tabs_bar->active_tab_index = 4;
                screen_tracks_set_album( &screen_tracks, navigation.selected_album_id );
                screen_tracks_refresh( &screen_tracks, &musicdb );

                input_t mouse_only = { input.mouse_x, input.mouse_y };
                screen_tracks_navigation_t nav;
                screen_tracks_update( &screen_tracks, &nav, true, &mouse_only );
                last_refresh = now;
                redraw = true;
                resize = true;
                }
            }
        else if( screen == SCREEN_SETTINGS )
            {
            screen_settings_navigation_t navigation;
            redraw |= screen_settings_update( &screen_settings, &navigation, resize, &input );
            }
        uint64_t new_counter = musicdb_change_counter( &musicdb );
        if( first_time_refresh || ( new_counter != db_change_counter && ( now - last_refresh >= 2 ) ) )
            {
            db_change_counter = new_counter;
            last_refresh = now;
            redraw = true;
            first_time_refresh = false;

            screen_genres_update_thumbs( &screen_genres, &musicdb );
            if( screen == SCREEN_GENRES )
                screen_genres_refresh( &screen_genres, &musicdb, remap_genres );
            else if( screen == SCREEN_ARTISTS )
                screen_artists_refresh( &screen_artists, &musicdb, remap_genres );
            else if( screen == SCREEN_ALBUMS )
                screen_albums_refresh( &screen_albums, &musicdb, remap_genres );
            else if( screen == SCREEN_TRACKS )
                screen_tracks_refresh( &screen_tracks, &musicdb );
            else if( screen == SCREEN_SHUFFLE )
                screen_shuffle_refresh( &screen_shuffle, &musicdb );
            else if( screen == SCREEN_SETTINGS )
                { }
            resize = true;
            }

        if( redraw )
            {
            redraw = false;
            render_new_frame( &render );

            if( draw_disable <= 0 )
                {
                
                if( screen != prev_screen ) 
                    {
                    input_t inp = { 0 };
                    if( screen == SCREEN_GENRES )
                        {
                        screen_genres_navigation_t navigation;
                        screen_genres_update( &screen_genres, &navigation, true, &inp );
                        }
                    else if( screen == SCREEN_ARTISTS )
                        {
                        screen_artists_navigation_t navigation;
                        screen_artists_update( &screen_artists, &navigation, true, &inp );
                        }
                    else if( screen == SCREEN_ALBUMS )
                        {
                        screen_albums_navigation_t navigation;
                        screen_albums_update( &screen_albums, &navigation, true, &inp );
                        }
                    else if( screen == SCREEN_TRACKS )
                        {
                        screen_tracks_navigation_t navigation;
                        screen_tracks_update( &screen_tracks, &navigation, true, &inp );
                        }
                    else if( screen == SCREEN_SHUFFLE )
                        { 
                        screen_shuffle_navigation_t navigation;
                        screen_shuffle_update( &screen_shuffle, &navigation, true, &inp, &musicdb, &control );
                        }
                    else if( screen == SCREEN_SETTINGS )
                        {
                        screen_settings_navigation_t navigation;
                        screen_settings_update( &screen_settings, &navigation, true, &inp );
                        }
                    }
                prev_screen = screen;

                bitmap_draw_tiled( &render, background );

                if( screen == SCREEN_GENRES )
                    redraw |= screen_genres_draw( &screen_genres );
                else if( screen == SCREEN_ARTISTS )
                    redraw |= screen_artists_draw( &screen_artists, &musicdb, remap_genres );
                else if( screen == SCREEN_ALBUMS )
                    redraw |= screen_albums_draw( &screen_albums, &musicdb, remap_genres );
                else if( screen == SCREEN_TRACKS )
                    redraw |= screen_tracks_draw( &screen_tracks );
                else if( screen == SCREEN_SHUFFLE )
                    redraw |= screen_shuffle_draw( &screen_shuffle, &musicdb, &control );
                else if( screen == SCREEN_SETTINGS )
                    redraw |= screen_settings_draw( &screen_settings );
                }

            tabs_bar_draw( tabs_bar );
            if( !first_frame && !applied_last_state_scroll )
                {
                applied_last_state_scroll = true;
                if( last_state.genres_scroll >= 0 && last_state.genres_scroll <= screen_genres.layout_max_scroll ) 
                    screen_genres.scroll = last_state.genres_scroll;
                if( last_state.artists_scroll >= 0 && last_state.artists_scroll <= screen_artists.layout_max_scroll ) 
                    screen_artists.scroll = last_state.artists_scroll;
                if( last_state.albums_scroll >= 0 && last_state.albums_scroll <= screen_albums.layout_max_scroll ) 
                    screen_albums.scroll = last_state.albums_scroll;
                redraw = true;
                }
            if( first_frame )
                {
                first_frame = false;
                render_new_frame( &render );
                redraw = true;
                }
            app_present( app, 0, 1, 1, 0xffffff, 0x000000 );
        }

        if( draw_disable > 0 ) --draw_disable;

        if( load_genre_thumbs )
            {
            while( screen_genres_update_thumbs( &screen_genres, &musicdb ) ) /* nothing */;
            load_genre_thumbs = false;
            redraw = true;
            }
        }
    
    app_screenmode( app, APP_SCREENMODE_WINDOW );
    FILE* fp = fopen( ".cache\\last_state.ini", "w" );
    fprintf( fp, "fullscreen=%d\n", g_fullscreen );   
    fprintf( fp, "maximized=%d\n", is_maximized( app ) );   
    maximize( app, false );
    fprintf( fp, "window_x=%d\n", app_window_x( app ) );
    fprintf( fp, "window_y=%d\n", app_window_y( app ) );
    fprintf( fp, "window_width=%d\n", app_window_width( app ) );
    fprintf( fp, "window_height=%d\n", app_window_height( app ) );
    fprintf( fp, "tab=%d\n", screen );
    fprintf( fp, "volume=%f\n", g_global_volume );
    fprintf( fp, "repeat=%d\n", control.repeat );
    fprintf( fp, "loop=%d\n", control.loop );
    fprintf( fp, "shuffle=%d\n", control.shuffle );
    fprintf( fp, "album=%u\n", control.album_id );
    fprintf( fp, "track=%u\n", control.track_index );
    fprintf( fp, "tracks_album=%u\n", screen_tracks.album_id );
    fprintf( fp, "genres_scroll=%u\n", screen_genres.scroll );
    fprintf( fp, "artists_scroll=%u\n", screen_artists.scroll );
    fprintf( fp, "albums_scroll=%u\n", screen_albums.scroll );
    fprintf( fp, "artists_genre_id=%u\n", screen_artists.current_genre_id );
    fprintf( fp, "artists_genre_thumb=%u\n", screen_artists.current_genre_thumb );
    fprintf( fp, "albums_genre_id=%u\n", screen_albums.current_genre_id );
    fprintf( fp, "albums_artist_id=%u\n", screen_albums.current_artist_id );
    fprintf( fp, "shuffle_list_index=%u\n", screen_shuffle.track_index );
    fclose( fp );

    hide_window( app );
    lbutton = false;
    //app_sound( app, 0, 0, 0 );
    ma_device_stop( &device );
    play_thread_destroy( play );
    ma_device_uninit(&device);

    frametimer_destroy( frametimer );

    background_tasks_term( &background_tasks );
    screen_settings_term( &screen_settings );
    screen_tracks_term( &screen_tracks );
    screen_albums_term( &screen_albums );
    screen_artists_term( &screen_artists );
    screen_genres_term( &screen_genres );
    tabs_bar_destroy( tabs_bar );

    bitmap_destroy( &render, slider_bitmaps.handle );
    bitmap_destroy( &render, slider_bitmaps.gap );
    bitmap_destroy( &render, slider_bitmaps.right );
    bitmap_destroy( &render, slider_bitmaps.middle );
    bitmap_destroy( &render, slider_bitmaps.left );
    bitmap_destroy( &render, variousartists);
    bitmap_destroy( &render, genredefault);
    bitmap_destroy( &render, frame);
    bitmap_destroy( &render, background);
    bitmap_destroy( &render, allalbums);
    bitmap_destroy( &render, albumdefault);
    bitmap_destroy( &render, allartists );

    for( int i = 0; i < sizeof( genres_mappings ) / sizeof( *genres_mappings ); ++i )
        {
        if( genres_mappings[ i ].original == NULL || genres_mappings[ i ].remapped == NULL ) break;
        free( (char*) genres_mappings[ i ].original );
        free( (char*) genres_mappings[ i ].remapped );
        }

    render_term( &render );
    gladLoaderUnloadGL();

    musicdb_close( &musicdb );
    img_destroy( g_shadow );
    tray_icon_off( app );
    return 0;
    }




unsigned int pow2_ceil( unsigned int x )
	{
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	++x;
	x += ( x == 0 );
	return x;
	}

img_t* img_create( char const* filename )
    {
    printf( "%s\n", filename );
    int w, h, c;
    stbi_uc* pixels = stbi_load( filename, &w, &h, &c, 4 );
    if( !pixels ) return 0;
    
    img_t* img = (img_t*) malloc( sizeof( img_t ) );
    img->pixels = (uint32_t*)pixels;
    img->width = w;
    img->height = h;
    return img;
    }


void img_destroy( img_t* img )
    {
    if( img->pixels ) stbi_image_free( (stbi_uc*) img->pixels );
    free( img );
    }


void img_resize( img_t* img, int width, int height ) 
    {
    uint32_t* pixels = (uint32_t*)malloc( width * height * sizeof( uint32_t ) );
    stbir_resize_uint8( (unsigned char*) img->pixels, img->width, img->height, 0, (unsigned char*) pixels, width, height, 0, 4 );
    free( img->pixels );
    img->pixels = pixels;
    img->width = width;
    img->height = height;
    }


uint32_t* make_mipmap_chain( char const* filename, int* width, int* height, int max_width, int max_height )
    {
    (void) max_width, max_height;
    struct stat s;
    stat( filename, &s );
    mmap_t* mmap = mmap_open_read_only( filename, (size_t) s.st_size );
    if( !mmap ) return 0;
    int w, h, c;
    stbi_uc* img = stbi_load_from_memory( (stbi_uc*) mmap_data( mmap ), (int) mmap_size( mmap ), &w, &h, &c, 4 );
    mmap_close( mmap );
    if( !img ) return 0; 
                
    int tw = w;
    int th = h;

    size_t newimg_size = 2;
    int temp_w = tw;
    int temp_h = th;
    while( temp_w > 0 && temp_h > 0 )
        {
        newimg_size += temp_w * temp_h;
        temp_w /= 2;
        temp_h /= 2;
        }
    newimg_size *= sizeof( uint32_t );

    uint32_t* newimg = (uint32_t*) malloc( newimg_size );
    uint32_t* ptr = newimg;
    *ptr++ = (uint32_t) w;
    *ptr++ = (uint32_t) h;
    
    while( tw > 0 && th > 0 )
        {
        stbir_resize_uint8( img, w, h, 0, (unsigned char*) ptr, tw, th, 0, 4 );
        ptr += tw * th;
        tw /= 2;
        th /= 2;
        }
    stbi_image_free( img );

    *width = w;
    *height = h;
    return newimg;
    }


void free_mipmap_chain( uint32_t* mipmaps )
    {
    free( mipmaps );
    }


bitmap_t* load_app_bitmap( render_t* render, char const* source_file, char const* cached_file, int max_width, int max_height )
    {
    bool regen = file_exists( source_file ) && file_exists( cached_file ) && file_more_recent( source_file, cached_file );

    struct stat s;
    stat( cached_file, &s );
    mmap_t* mmap = regen ? 0 : mmap_open_read_only( cached_file, (size_t) s.st_size );
    if( mmap )
        {
        bitmap_t* bitmap = 0;
        uint32_t* ptr = (uint32_t*) mmap_data( mmap );
        int width = (int) *ptr++;
        int height = (int) *ptr++;

        int tw = width;
        int th = height;

        bitmap = bitmap_create( render, width, height, ptr, tw, th );       
        mmap_close( mmap );
        return bitmap;
        }
    else
        {
        bitmap_t* bitmap = 0;
        int width, height;
        uint32_t* pixels = make_mipmap_chain( source_file, &width, &height, max_width, max_height );
        if( pixels )
            {
            int tw = width;
            int th = height;


            bitmap = bitmap_create( render, width, height, pixels + 2, tw, th);                
            size_t out_size = 2;
            while( tw > 0 && th > 0 )
                {
                out_size += tw * th;
                tw /= 2;
                th /= 2;
                }
            out_size *= sizeof( uint32_t );
            create_path( cdirname( cached_file ) );
            file_save_data( pixels, out_size, cached_file, FILE_MODE_BINARY );
            free( pixels );
            }
        return bitmap;
        }
    }


#define  APP_IMPLEMENTATION
#define  APP_WINDOWS
#include "libs/app.h"

void opengl_preinit( void );
void show_splash_screen( void );
	
#pragma comment( lib, "ole32.lib" )


int main( int argc, char** argv )
    {
	#ifndef NDEBUG
		int flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ); // Get current flag
		flag ^= _CRTDBG_LEAK_CHECK_DF; // Turn on leak-checking bit
		_CrtSetDbgFlag( flag ); // Set flag to the new value
        //_CrtSetBreakAlloc( 0 );
	#endif

    HMODULE shcore = LoadLibraryA( "shcore.dll" );
    if( shcore ) {
        HRESULT (__stdcall *Win32SetProcessDpiAwareness)( int ) = (HRESULT (__stdcall *)( int ))GetProcAddress( shcore, "SetProcessDpiAwareness" );
        if( Win32SetProcessDpiAwareness ) {
            #define WIN32_PROCESS_PER_MONITOR_DPI_AWARE 2
            Win32SetProcessDpiAwareness( WIN32_PROCESS_PER_MONITOR_DPI_AWARE );
        }
        FreeLibrary( shcore );
    }

    CoInitializeEx( NULL, COINIT_MULTITHREADED );

    opengl_preinit();
    show_splash_screen();

    for( int i = 1; i < argc; ++i ) 
        {
        if( strcmp( argv[ i ], "--noscan" )== 0 )
            {
            g_noscan = true;
            }
        else if( strcmp( argv[ i ], "--fullscreen" )== 0 )
            {
            g_fullscreen = true;
            }
        }

    (void) argc, argv;
    int result = app_run( app_proc, NULL, NULL, NULL, NULL );
    CoUninitialize();
    return result;
    }

// pass-through so the program will build with either /SUBSYSTEM:WINDOWS or /SUBSYSTEN:CONSOLE
#ifdef __cplusplus
    extern "C" int __stdcall WinMain( struct HINSTANCE__*, struct HINSTANCE__*, char*, int ) { return main( __argc, __argv ); }
#else
    struct HINSTANCE__;
    int __stdcall WinMain( struct HINSTANCE__* a, struct HINSTANCE__* b, char* c, int d ) { (void) a, b, c, d; return main( __argc, __argv ); }
#endif
    
#define DIR_IMPLEMENTATION
#define DIR_WINDOWS
#include "libs/dir.h"

#define FILE_IMPLEMENTATION
#include "libs/file.h"

#define FILE_UTIL_IMPLEMENTATION
#include "libs/file_util.h"

#define FILEWATCH_IMPLEMENTATION
#include "libs/filewatch.h"

#define FRAMETIMER_IMPLEMENTATION
#include "libs/frametimer.h"

#define GLAD_GL_IMPLEMENTATION
#include "libs/glad.h"

#define ID3TAG_IMPLEMENTATION
#include "libs/id3tag.h"

#define INI_IMPLEMENTATION
#include "libs/ini.h"

#define MINIMP3_IMPLEMENTATION
#pragma warning( push )
#pragma warning( disable: 4242 ) // '=': conversion from 'int' to 'uint8_t', possible loss of data
#pragma warning( disable: 4244 ) // '=': conversion from 'uint32_t' to 'uint8_t', possible loss of data
#pragma warning( disable: 4255 )
#pragma warning( disable: 4267 ) // 'argument': conversion from 'size_t' to 'int', possible loss of data
#pragma warning( disable: 4365 ) // 'initializing': conversion from 'int' to 'uint32_t', signed/unsigned mismatch
#pragma warning( disable: 4668 )
#pragma warning( disable: 4706 ) //  assignment within conditional expression
#include "libs/minimp3.h"
#include "libs/minimp3_ex.h"
#pragma warning( pop )

#define MMAP_IMPLEMENTATION
#include "libs/mmap.h"

#define MINIAUDIO_IMPLEMENTATION
#pragma warning( push )
#pragma warning( disable: 4061 )
#pragma warning( disable: 4191 )
#pragma warning( disable: 4255 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4668 )
#include "libs/miniaudio.h"
#pragma warning( pop )

#define RND_IMPLEMENTATION
#include "libs/rnd.h"

#define STB_IMAGE_IMPLEMENTATION
#pragma warning( push )
#pragma warning( disable: 4244 ) // '=': conversion from 'uint32_t' to 'uint8_t', possible loss of data
#pragma warning( disable: 4296 )
#pragma warning( disable: 4365 ) // 'initializing': conversion from 'int' to 'uint32_t', signed/unsigned mismatch
#pragma warning( disable: 4456 ) // declaration of 'k' hides previous local declaration
#include "libs/stb_image.h"
#pragma warning( pop )

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#pragma warning( push )
#pragma warning( disable: 4365 ) // 'initializing': conversion from 'int' to 'uint32_t', signed/unsigned mismatch
#include "libs/stb_image_resize.h"
#pragma warning( pop )

#define STB_TRUETYPE_IMPLEMENTATION
#pragma warning( push )
#pragma warning( disable: 4365 ) // 'initializing': conversion from 'int' to 'uint32_t', signed/unsigned mismatch
#include "libs/stb_truetype.h"
#pragma warning( pop )

#define THREAD_IMPLEMENTATION
#include "libs/thread.h"

#define VECMATH_IMPLEMENTATION
#pragma warning( push )
#pragma warning( disable: 4244 )
#include "libs/vecmath.h"
#pragma warning( pop )


#pragma warning( push )
#pragma warning( disable: 4242 )
#pragma warning( disable: 4244 )
#include "libs/wyhash.h"
#pragma warning( pop )


uint64_t calc_hash( void const* data, size_t size ) {
   return wyhash( data, size, 0, _wyp );
}

void copy_to_clipboard( char const* str )
    {
    const size_t len = strlen( str ) + 1;
    HGLOBAL hMem =  GlobalAlloc( GMEM_MOVEABLE, len );
    memcpy( GlobalLock( hMem ), str, len );
    GlobalUnlock( hMem );
    OpenClipboard( 0 );
    EmptyClipboard();
    SetClipboardData( CF_TEXT, hMem );
    CloseClipboard();
    }


int copy_from_clipboard( char* str, size_t capacity )
    {
    if( !OpenClipboard( 0 ) ) return 0;

    HANDLE h = GetClipboardData( CF_TEXT );
    int len = 0;
    if( h )
        {
        len = (int)strlen( (char const*) h );
        if( len > 0 && str ) 
            {
            strncpy( str, (char const*) h, capacity );
            str[ capacity - 1 ] = '\0';
            }
        }
    CloseClipboard();
    return len;
    }



#include <shellapi.h>
#include <wtsapi32.h>
#pragma comment( lib, "wtsapi32.lib" )

static HMENU hPopMenu;
static bool g_is_computer_locked = false;

void toggle_screenmode( app_t* app ) 
    {
    if( g_fullscreen ) 
        {
        SetForegroundWindow(app->hwnd);
        ShowWindow( app->hwnd, SW_SHOWNORMAL );
        app_screenmode( app, APP_SCREENMODE_WINDOW );
		g_fullscreen = FALSE;
        }
    else
        {
        app_screenmode( app, APP_SCREENMODE_FULLSCREEN );
        SetForegroundWindow(app->hwnd);
        ShowWindow( app->hwnd, SW_SHOWNORMAL );
		g_fullscreen = TRUE;
        }
    }


#define IDM_OPEN 1
#define IDM_WINDOW 2
#define IDM_FULLSCREEN 3
#define IDM_SEPARATOR 4
#define IDM_QUIT 5

LRESULT CALLBACK tray_wndproc( app_t* app, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam )
    {
    switch( message )
        {
        case WM_USER:
            {
            switch( LOWORD( lparam ) ) 
                {
                case WM_RBUTTONDOWN:
                    {
                    POINT lpClickPoint;
			        GetCursorPos(&lpClickPoint);
			        hPopMenu = CreatePopupMenu();
			        InsertMenuW(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING/*|MF_GRAYED*/,IDM_OPEN,(L"Open"));
			        if ( g_fullscreen == TRUE )
				        InsertMenuW(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_WINDOW,(L"Window"));
			        else 
				        InsertMenuW(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_FULLSCREEN,(L"Fullscreen"));
			        InsertMenuW(hPopMenu,0xFFFFFFFF,MF_SEPARATOR,IDM_SEPARATOR,(L"---"));
			        InsertMenuW(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_QUIT,(L"Quit"));
									
			        SetForegroundWindow(hwnd);
			        TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
			        return TRUE; 
                    } break;
                case WM_LBUTTONDOWN:
                    SetForegroundWindow(app->hwnd);
                    ShowWindow( app->hwnd, SW_SHOWNORMAL );
                    return TRUE; 
                    break;
                }
            }
        case WM_COMMAND:
            {
		    switch( LOWORD( wparam ) )
    		    {
                case IDM_OPEN:
                    SetForegroundWindow(app->hwnd);
                    ShowWindow( app->hwnd, SW_SHOWNORMAL );
                    return TRUE; 
                    break;
			    case IDM_WINDOW:
                    SetForegroundWindow(app->hwnd);
                    ShowWindow( app->hwnd, SW_SHOWNORMAL );
                    app_screenmode( app, APP_SCREENMODE_WINDOW );
				    g_fullscreen = FALSE;
				    break;
			    case IDM_FULLSCREEN:
                    app_screenmode( app, APP_SCREENMODE_FULLSCREEN );
                    SetForegroundWindow(app->hwnd);
                    ShowWindow( app->hwnd, SW_SHOWNORMAL );
				    g_fullscreen = TRUE;
				    break;
			    case IDM_QUIT:
				    app->closed = TRUE;
				    break;
	    	    }
            }

        case WM_WTSSESSION_CHANGE:
			if( wparam == WTS_SESSION_LOCK )
                g_is_computer_locked = true;
            else if( wparam == WTS_SESSION_UNLOCK )
                g_is_computer_locked = false;
    		break;
        }
    return DefWindowProc( hwnd, message, wparam, lparam );
    }


void tray_icon( app_t* app )
    {
    NOTIFYICONDATAW tray = { 0 };
    tray.cbSize = sizeof( NOTIFYICONDATA ); 
    tray.hWnd = app->hwnd;
    tray.uID = 0; 
    tray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; 
    tray.hIcon = app->icon; 
    tray.uCallbackMessage = WM_USER; 
    wcscpy( tray.szTip, L"I \u2764 Music" );
    app->user_wndproc = tray_wndproc;
    
    Shell_NotifyIconW( NIM_ADD, &tray );

    // TODO: move this to its own function
    WTSRegisterSessionNotification( app->hwnd, NOTIFY_FOR_ALL_SESSIONS );
    }


bool computer_locked( void )
    {
    return g_is_computer_locked;
    }


void tray_tooltip( app_t* app, char const* tooltip )
    {
    if( !tooltip ) tooltip = "";
    NOTIFYICONDATAW tray = { 0 };
    tray.cbSize = sizeof( NOTIFYICONDATA ); 
    tray.hWnd = app->hwnd;
    tray.uID = 0; 
    tray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; 
    tray.hIcon = app->icon; 
    tray.uCallbackMessage = WM_USER; 
    WCHAR wtooltip[ 256 ];
    MultiByteToWideChar( CP_ACP, 0, tooltip, -1, wtooltip, 256 );
    wcscpy( tray.szTip, L"I \u2764 Music" );
    wcscat( tray.szTip, wtooltip );
    app->user_wndproc = tray_wndproc;
    
    Shell_NotifyIconW( NIM_MODIFY, &tray );

    // TODO: move this to its own function
    WTSRegisterSessionNotification( app->hwnd, NOTIFY_FOR_ALL_SESSIONS );
    }



void tray_icon_off( app_t* app )
    {
    NOTIFYICONDATAW tray = { 0 };
    tray.cbSize = sizeof( NOTIFYICONDATA ); 
    tray.hWnd = app->hwnd;
    tray.uID = 0; 
    tray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; 
    tray.hIcon = app->icon; 
    tray.uCallbackMessage = WM_USER; 
    wcscpy( tray.szTip, L"I \u2764 Music" );

    Shell_NotifyIconW( NIM_DELETE, &tray );
    }


void show_window( app_t* app )
    {
    SetForegroundWindow(app->hwnd);
    ShowWindow( app->hwnd, SW_SHOWNORMAL );
    }

void hide_window( app_t* app )
    {
    ShowWindow( app->hwnd, SW_MINIMIZE );
    ShowWindow( app->hwnd, SW_HIDE );
    }


int opengl_preinit_thread_proc( void* user_data ) {
    (void) user_data;
    HDC dc = GetDC(NULL);
	DescribePixelFormat(dc, 0, 0, NULL);
	ReleaseDC(NULL, dc);
    return 0;
}


void opengl_preinit( void ) {
    thread_create( opengl_preinit_thread_proc, NULL, 0, THREAD_STACK_SIZE_DEFAULT );
}


bool lbutton_down( void ) {
    return ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) > 0;
}


bool bring_up_from_minimized( app_t* app ) {
    if( ( GetAsyncKeyState( VK_F12 ) & 0x8000 ) > 0 && 
        ( ( GetAsyncKeyState( VK_LWIN ) & 0x8000 ) > 0 
         || ( GetAsyncKeyState( VK_RWIN ) & 0x8000 ) > 0 ) ){

        SetForegroundWindow(app->hwnd);
        ShowWindow( app->hwnd, SW_SHOWNORMAL );
        return true;
    }
    return false;
}

#include "logo_image.h"


uint32_t blendpix( uint32_t color1, uint32_t color2, int alpha )
	{
	int inv_alpha = 255 - alpha;
	return
        (color1 & 0xff000000) |
		(((((color1 & 0x00ff0000)>>16)*inv_alpha)+((color2 & 0x00ff0000)>>16)*alpha)>>8) |
		(((((color1 & 0x0000ff00)>>8 )*inv_alpha)+((color2 & 0x0000ff00)>>8 )*alpha)>>8)<<8  |
		(((((color1 & 0x000000ff)    )*inv_alpha)+((color2 & 0x000000ff)    )*alpha)>>8)<< 16
        ;
	}


int splash_screen_thread_proc( void* user_data ) {
    (void) user_data;
    int w, h, c;
    stbi_uc* img = stbi_load_from_memory( logo_image, sizeof( logo_image ), &w, &h, &c, 4 );

    RECT r;
    GetClientRect( GetDesktopWindow(), &r );

    int width = ( r.right - r.left ) / 2;
    int height = ( width * h ) /w;
    int x = r.left + ( ( r.right - r.left ) - width ) / 2;
    int y = r.top + ( ( r.bottom - r.top ) - height ) / 2;

    WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_DBLCLKS | CS_OWNDC,  
        (WNDPROC) DefWindowProc, 0, 0, 0, 0, 0, 0, 0, TEXT( "splashscreen_wc" ), 0 };
    wc.hInstance = GetModuleHandleA( NULL); 
    wc.hIcon = LoadIconA( wc.hInstance, MAKEINTRESOURCEA( 1 ) ); 
    wc.hIconSm = wc.hIcon;
    wc.hCursor = NULL; 
    wc.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH ); 
    RegisterClassEx( &wc );
    HWND hwnd = CreateWindowEx( 0, wc.lpszClassName, 0, WS_POPUP,
        x - 1, y - 1, width + 2, height + 2, (HWND) 0, (HMENU) 0, wc.hInstance, 0 );
    ShowWindow( hwnd, SW_SHOWNORMAL );

    HDC hdc = GetDC( hwnd );
    BITMAPINFO bmi;
    bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = 0;
    bmi.bmiHeader.biYPelsPerMeter = 0;
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;

    uint32_t* scaled = (uint32_t*) malloc( sizeof( uint32_t ) * width * height );
    stbir_resize_uint8( (unsigned char*) img, w, h, 0, (unsigned char*) scaled, width, height, 0, 4 );
    free( img );
    for( int i = 0; i < width * height; ++i ) {
        scaled[ i ] = blendpix( 0xffffffff, scaled[ i ], scaled[ i ] >> 24 );
    }
    SetDIBitsToDevice( hdc, 1, 1, width, height, 0, 0, 0, height, scaled, &bmi, 0 );
    free( scaled );

    RECT bounds;
    bounds.left = 0;
    bounds.right = width;
    bounds.top = height - 24;
    bounds.bottom = height;
    DrawTextA( hdc, "Preparing application for first time use. Next time we will start faster.", -1, &bounds, DT_CENTER );

    for( ; ; ) {
        MSG msg;
        while( PeekMessage( &msg, hwnd, 0, 0, PM_NOREMOVE ) ) {
            if( g_init_done ) {
                DestroyWindow( hwnd );
                g_splash_screen_showing  = false;
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);            
        }
    }
}


void show_splash_screen( void ) {
    if( folder_exists( ".cache" ) ) return;
    g_splash_screen_showing = true;
    thread_create( splash_screen_thread_proc, NULL, 0, THREAD_STACK_SIZE_DEFAULT );
}


bool is_maximized( app_t* app )
    {
    return IsZoomed( app->hwnd );
    }

void maximize( app_t* app, bool zoom ) 
    {
    ShowWindow( app->hwnd, zoom ? SW_MAXIMIZE : SW_SHOWNORMAL );
    }
