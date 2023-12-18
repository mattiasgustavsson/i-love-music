#ifdef _M_IX86
    #define MUSICDB_THUMBNAIL_FILES
#endif 

static int MUSICDB_VERSION = 3;

#define MUSICDB_HEADER_ID_SIZE 32
static char const musicdb_genres_header_id[ MUSICDB_HEADER_ID_SIZE ] =  "ilm_db_genres";
static char const musicdb_artists_header_id[ MUSICDB_HEADER_ID_SIZE ] = "ilm_db_artists";
static char const musicdb_albums_header_id[ MUSICDB_HEADER_ID_SIZE ] =  "ilm_db_albums";
static char const musicdb_songs_header_id[ MUSICDB_HEADER_ID_SIZE ] =   "ilm_db_songs";
static char const musicdb_shuffle_header_id[ MUSICDB_HEADER_ID_SIZE ] = "ilm_db_shuffle";
static char const musicdb_mixtapes_header_id[ MUSICDB_HEADER_ID_SIZE ] = "ilm_db_mixtapes";
static char const musicdb_thumbnails_header_id[ MUSICDB_HEADER_ID_SIZE ] = "ilm_db_thumbnails";
#ifndef MUSICDB_THUMBNAIL_FILES
static char const musicdb_pixels_header_id[ MUSICDB_HEADER_ID_SIZE ] = "ilm_db_pixels";
#endif

typedef struct musicdb_header_t
    {
    char header_id[ MUSICDB_HEADER_ID_SIZE ];
    int version;
    size_t size;

    int capacity;
    int count;
    } musicdb_header_t;


// genres.db

typedef struct musicdb_genre_t
    {
    uint32_t id;
    char label[ 64 ];
    char remapped_label[ 64 ];
    uint32_t thumbnail_id;
    uint32_t remapped_thumb_id;
    int album_count;
    } musicdb_genre_t;


typedef struct musicdb_genres_t
    {
    musicdb_header_t header;
    musicdb_genre_t items[ 1 ]; // "open" array
    } musicdb_genres_t;


// artists.db

typedef struct musicdb_artist_t
    {
    uint32_t id;
    char name[ 64 ];
    int album_count;
    uint32_t thumb_id;
    int thumb_album_count;
    } musicdb_artist_t;


typedef struct musicdb_artists_t
    {
    musicdb_header_t header;
    musicdb_artist_t items[ 1 ]; // "open" array
    } musicdb_artists_t;


// albums.db

typedef struct musicdb_album_t
    {
    uint32_t id;
    uint32_t artist_id;
    char artist[ 128 ];
    char album_artist[ 128 ];
    char sort_artist[ 128 ];
    char title[ 128 ];
    uint32_t thumb_id;
    int year;
    int disc_count;
    int track_count;
    bool compilation;
    bool various;
    char path[ 256 ];

    int genres_count;
    uint32_t genres_id[ 16 ];
    } musicdb_album_t;

typedef struct musicdb_albums_t
    {
    musicdb_header_t header;
    musicdb_album_t items[ 1 ]; // "open" array
    } musicdb_albums_t;


// songs.db

typedef struct musicdb_song_t
    {
    uint32_t id;
    char title[ 128 ];
    char album[ 128 ];
    char artist[ 128 ];
    char album_artist[ 128 ];
    char sort_album_artist[ 128 ];
    char genre[ 64];
    uint32_t album_id;
    uint32_t mixtape_id;
    int length_in_seconds;
    int disc_number;
    int track_number;
    int year;
    bool compilation;
    float track_gain;
    float album_gain;

    char filename[ 256 ];
    size_t filesize;
    time_t last_changed;
    } musicdb_song_t;


typedef struct musicdb_songs_t
    {
    musicdb_header_t header;
    musicdb_song_t items[ 1 ]; // "open" array
    } musicdb_songs_t;


// shuffle.db

typedef struct musicdb_shuffle_t
    {
    musicdb_header_t header;
    int items[ 1 ]; // "open" array of indices into musicdb_songs_t
    } musicdb_shuffle_t;


// mixtapes.db

typedef struct musicdb_mixtape_t
    {
    uint32_t id;
    char title[ 128 ];
    uint32_t thumb_id;
    int year;
    int track_count;
    } musicdb_mixtape_t;


typedef struct musicdb_mixtapes_t
    {
    musicdb_header_t header;
    musicdb_mixtape_t items[ 1 ]; // "open" array
    } musicdb_mixtapes_t;


// thumbnails.db

typedef struct musicdb_thumbnail_t
    {
    uint32_t id;
    char source_filename[ 260 ];
    time_t last_modified;
    int source_image_index;
    bool initialized;
    int width;
    int height;
    #ifdef MUSICDB_THUMBNAIL_FILES
        char thumbs_file[ 260 ];
    #else
        size_t pixels_offset;
    #endif
    } musicdb_thumbnail_t;


typedef struct musicdb_thumbnails_t
    {
    musicdb_header_t header;
    musicdb_thumbnail_t items[ 1 ]; // "open" array
    } musicdb_thumbnails_t;


#ifndef MUSICDB_THUMBNAIL_FILES

// pixels.db

typedef struct musicdb_pixels_t
    {
    musicdb_header_t header;
    size_t next_free;
    uint32_t pixel_data[ 1 ]; // "open" array
    } musicdb_pixels_t;

#endif


typedef struct music_db_t
    {
    background_tasks_t* background_tasks;
    filewatch_t* filewatches[ 32 ];
    thread_mutex_t mutex;

    char path[ 256 ];

    musicdb_genres_t* genres;
    musicdb_artists_t* artists;
    musicdb_albums_t* albums;
    musicdb_songs_t* songs;
    musicdb_shuffle_t* shuffle;
    musicdb_mixtapes_t* mixtapes;
    musicdb_thumbnails_t* thumbnails;
    #ifndef MUSICDB_THUMBNAIL_FILES
        musicdb_pixels_t* pixels;
    #endif

    mmap_t* mmap_genres;
    mmap_t* mmap_artists;
    mmap_t* mmap_albums;
    mmap_t* mmap_songs;
    mmap_t* mmap_shuffle;
    mmap_t* mmap_mixtapes;
    mmap_t* mmap_thumbnails;
    #ifndef MUSICDB_THUMBNAIL_FILES
        mmap_t* mmap_pixels;
    #endif

    int songs_added;
    uint64_t change_counter;


    #ifndef MUSICDB_THUMBNAIL_FILES
        mmap_t* preload_mmap_pixels;
        thread_atomic_int_t preload_exit_flag;
        thread_ptr_t preload_thread;
    #endif
    } music_db_t;


uint32_t const MUSICDB_INVALID_ID = 0xffffffff;


#define MUSICDB_INTERNAL_STRNCPY( dst, src ) strncpy( (dst), (src), sizeof( dst ) ); (dst)[ sizeof( dst ) - 1 ] = '\0';


mmap_t* musicdb_internal_mmap_create( char const* filename, char const* header_id, size_t entry_size, int capacity )
    {
    size_t size = sizeof( musicdb_header_t ) + entry_size * capacity;
    mmap_t* mmap = mmap_create( filename, size );
    if( !mmap) return 0;

    musicdb_header_t* header = (musicdb_header_t*) mmap_data( mmap );
    if( !header) return 0;

    memset( header->header_id, 0,  MUSICDB_HEADER_ID_SIZE );
    strcpy( header->header_id, header_id );
    header->version = MUSICDB_VERSION;
    header->size = size;
    header->capacity = capacity;
    header->count = 0;
    return mmap;        
    }


mmap_t* musicdb_internal_mmap_open(  char const* filename, char const* header_id )
    {
    mmap_t* header_mmap = mmap_open( filename, sizeof( musicdb_header_t ) );
    if( !header_mmap ) return 0;

    musicdb_header_t* header = (musicdb_header_t*) mmap_data( header_mmap );
    if( !header || strcmp( header->header_id, header_id ) != 0 || header->version != MUSICDB_VERSION )
        {
        mmap_close( header_mmap );
        return 0;
        }

    size_t size = header->size;
    mmap_close( header_mmap );

    return mmap_open( filename, size );        
    }


mmap_t* musicdb_internal_mmap_resize( mmap_t* mmap, size_t entry_size, int capacity )
    {
    size_t size = sizeof( musicdb_header_t ) + entry_size * capacity;
    mmap_t* new_mmap = mmap_open( mmap_filename( mmap ), size );
    if( !new_mmap) return 0;
    mmap_close( mmap );

    musicdb_header_t* header = (musicdb_header_t*) mmap_data( new_mmap );
    if( !header) return 0;

    header->size = size;
    header->capacity = capacity;
    return new_mmap;        
    }

#ifndef MUSICDB_THUMBNAIL_FILES

mmap_t* musicdb_internal_mmap_create_pixels( char const* filename, char const* header_id, size_t new_size )
    {
    size_t size = sizeof( musicdb_pixels_t ) + new_size;
    mmap_t* mmap = mmap_create( filename, size );
    if( !mmap) return 0;

    musicdb_header_t* header = (musicdb_header_t*) mmap_data( mmap );
    if( !header) return 0;

    memset( header->header_id, 0,  MUSICDB_HEADER_ID_SIZE );
    strcpy( header->header_id, header_id );
    header->version = MUSICDB_VERSION;
    header->size = size;
    header->capacity = 0;
    header->count = 0;
    ((musicdb_pixels_t*) header)->next_free = 1;
    return mmap;        
    }


mmap_t* musicdb_internal_mmap_resize_pixels( mmap_t* mmap, size_t new_size )
    {
    size_t size = sizeof( musicdb_pixels_t ) + new_size;
    mmap_t* new_mmap = mmap_open( mmap_filename( mmap ), size );
    if( !new_mmap) return 0;
    mmap_close( mmap );

    musicdb_header_t* header = (musicdb_header_t*) mmap_data( new_mmap );
    if( !header) return 0;

    header->size = size;
    header->capacity = 0;
    return new_mmap;        
    }


int preload_pixel_cache_thread_proc( void* user_data )
    {
    music_db_t* db = (music_db_t*)user_data;
    volatile uint8_t accumulator = 0;
    uint8_t* ptr = (uint8_t*)mmap_data( db->preload_mmap_pixels );
    uint8_t* end = ptr + mmap_size( db->preload_mmap_pixels );
    while( ptr < end && thread_atomic_int_load( &db->preload_exit_flag ) == 0 )
        {
        accumulator ^= *ptr;
        ptr += 4096;
        }
    mmap_close( db->preload_mmap_pixels );
    printf( "Pixel preload done\n" );
    return 0;
    }


void preload_pixel_cache( music_db_t* db, char const* pixels_filename ) 
    {
    db->preload_mmap_pixels = musicdb_internal_mmap_open( pixels_filename, musicdb_pixels_header_id );
    thread_atomic_int_store( &db->preload_exit_flag, 0 );
    db->preload_thread = thread_create( preload_pixel_cache_thread_proc, db, 0, THREAD_STACK_SIZE_DEFAULT );
    }

#endif



void musicdb_close( music_db_t* db )
    {
    #ifndef MUSICDB_THUMBNAIL_FILES
    thread_atomic_int_store( &db->preload_exit_flag, 1 );
    thread_join( db->preload_thread );
    thread_destroy( db->preload_thread );
    if( db->mmap_pixels ) mmap_close( db->mmap_pixels );
    #endif
    if( db->mmap_thumbnails ) mmap_close( db->mmap_thumbnails );
    if( db->mmap_mixtapes ) mmap_close( db->mmap_mixtapes );
    if( db->mmap_shuffle ) mmap_close( db->mmap_shuffle );
    if( db->mmap_songs ) mmap_close( db->mmap_songs );
    if( db->mmap_albums ) mmap_close( db->mmap_albums );
    if( db->mmap_artists ) mmap_close( db->mmap_artists );
    if( db->mmap_genres ) mmap_close( db->mmap_genres );
    for( int i = 0; i < sizeof( db->filewatches ) / sizeof( *db->filewatches ); ++i )
        if( db->filewatches[ i ] ) filewatch_destroy( db->filewatches[ i ] );
    thread_mutex_term( &db->mutex );
    }


bool musicdb_open( music_db_t* db, background_tasks_t* background_tasks, char const* path, bool force_regenerate )
    {
    memset( db, 0, sizeof( *db ) );

    db->background_tasks = background_tasks;
    thread_mutex_init( &db->mutex );

    create_path( path );
    if( !folder_exists( path ) ) return false;
    strcpy( db->path, path );

    char genres_filename[ 256 ] = "";
    char artists_filename[ 256 ] = "";
    char albums_filename[ 256 ] = "";
    char songs_filename[ 256 ] = "";
    char shuffle_filename[ 256 ] = "";
    char mixtapes_filename[ 256 ] = "";
    char thumbnails_filename[ 256 ] = "";
    #ifndef MUSICDB_THUMBNAIL_FILES
    char pixels_filename[ 256 ] = "";
    #endif
    
    strcpy( genres_filename, path );
    strcpy( artists_filename, path );
    strcpy( albums_filename, path );
    strcpy( songs_filename, path );
    strcpy( shuffle_filename, path );
    strcpy( mixtapes_filename, path );
    strcpy( thumbnails_filename, path );
    #ifndef MUSICDB_THUMBNAIL_FILES
    strcpy( pixels_filename, path );
    #endif

    if( *path && path[ strlen( path ) - 1 ] != '/' && path[ strlen( path ) - 1 ] != '\\' )
        {
        strcat( genres_filename, "\\" );
        strcat( artists_filename, "\\" );
        strcat( albums_filename, "\\" );
        strcat( songs_filename, "\\" );
        strcat( shuffle_filename, "\\" );
        strcat( mixtapes_filename, "\\" );
        strcat( thumbnails_filename, "\\" );
    #ifndef MUSICDB_THUMBNAIL_FILES
        strcat( pixels_filename, "\\" );
    #endif
        }

    strcat( genres_filename, "genres.db" );
    strcat( artists_filename, "artists.db" );
    strcat( albums_filename, "albums.db" );
    strcat( songs_filename, "songs.db" );
    strcat( shuffle_filename, "shuffle.db" );
    strcat( mixtapes_filename, "mixtapes.db" );
    strcat( thumbnails_filename, "thumbnails.db" );
    #ifndef MUSICDB_THUMBNAIL_FILES
    strcat( pixels_filename, "pixels.db" );
    #endif

    bool regenerate_db = force_regenerate;
    if( !file_exists( genres_filename ) ) regenerate_db = true;
    if( !file_exists( artists_filename ) ) regenerate_db = true;
    if( !file_exists( albums_filename ) ) regenerate_db = true;
    if( !file_exists( songs_filename ) ) regenerate_db = true;
    if( !file_exists( shuffle_filename ) ) regenerate_db = true;
    if( !file_exists( mixtapes_filename ) ) regenerate_db = true;
    if( !file_exists( thumbnails_filename ) ) regenerate_db = true;
    #ifndef MUSICDB_THUMBNAIL_FILES
    if( !file_exists( pixels_filename ) ) regenerate_db = true;
    #endif

    if( regenerate_db )
        {
        db->mmap_genres = musicdb_internal_mmap_create( genres_filename, musicdb_genres_header_id, sizeof( musicdb_genre_t ), 256 );           
        db->mmap_artists = musicdb_internal_mmap_create( artists_filename, musicdb_artists_header_id, sizeof( musicdb_artist_t ), 1024 );
        db->mmap_albums = musicdb_internal_mmap_create( albums_filename, musicdb_albums_header_id, sizeof( musicdb_album_t ), 4096 );
        db->mmap_songs = musicdb_internal_mmap_create( songs_filename, musicdb_songs_header_id, sizeof( musicdb_song_t ), 16384 );
        db->mmap_shuffle = musicdb_internal_mmap_create( shuffle_filename, musicdb_shuffle_header_id, sizeof( int ), 8192 );
        db->mmap_mixtapes = musicdb_internal_mmap_create( mixtapes_filename, musicdb_mixtapes_header_id, sizeof( musicdb_mixtape_t ), 2048 );
        db->mmap_thumbnails = musicdb_internal_mmap_create( thumbnails_filename, musicdb_thumbnails_header_id, sizeof( musicdb_thumbnail_t ), 4096 );
    #ifndef MUSICDB_THUMBNAIL_FILES
        db->mmap_pixels = musicdb_internal_mmap_create_pixels( pixels_filename, musicdb_pixels_header_id, 32 * 1024 * 1024 );
    #endif
        // TODO: store regenerate flag
        }
    else
        {
        db->mmap_genres = musicdb_internal_mmap_open( genres_filename, musicdb_genres_header_id );
        db->mmap_artists = musicdb_internal_mmap_open( artists_filename, musicdb_artists_header_id );
        db->mmap_albums = musicdb_internal_mmap_open( albums_filename, musicdb_albums_header_id );
        db->mmap_songs = musicdb_internal_mmap_open( songs_filename, musicdb_songs_header_id );
        db->mmap_shuffle = musicdb_internal_mmap_open( shuffle_filename, musicdb_shuffle_header_id );
        db->mmap_mixtapes = musicdb_internal_mmap_open( mixtapes_filename, musicdb_mixtapes_header_id );
        db->mmap_thumbnails = musicdb_internal_mmap_open( thumbnails_filename, musicdb_thumbnails_header_id );
    #ifndef MUSICDB_THUMBNAIL_FILES
        db->mmap_pixels = musicdb_internal_mmap_open( pixels_filename, musicdb_pixels_header_id );
    #endif
        }

    if( !db->mmap_genres || !db->mmap_artists || !db->mmap_albums || !db->mmap_songs || !db->mmap_shuffle || !db->mmap_mixtapes || !db->mmap_thumbnails 
    #ifndef MUSICDB_THUMBNAIL_FILES
        || !db->mmap_pixels 
    #endif
        ) 
        { 
        musicdb_close( db ); 
        return false;
        }

    db->genres = (musicdb_genres_t*) mmap_data( db->mmap_genres );
    db->artists = (musicdb_artists_t*) mmap_data( db->mmap_artists );
    db->albums = (musicdb_albums_t*) mmap_data( db->mmap_albums );
    db->songs = (musicdb_songs_t*) mmap_data( db->mmap_songs );
    db->shuffle = (musicdb_shuffle_t*) mmap_data( db->mmap_shuffle );
    db->mixtapes = (musicdb_mixtapes_t*) mmap_data( db->mmap_mixtapes );
    db->thumbnails = (musicdb_thumbnails_t*) mmap_data( db->mmap_thumbnails );
    #ifndef MUSICDB_THUMBNAIL_FILES
    db->pixels = (musicdb_pixels_t*) mmap_data( db->mmap_pixels );
    #endif

    if( !db->genres || !db->artists || !db->albums || !db->songs || !db->shuffle || !db->mixtapes || !db->thumbnails 
    #ifndef MUSICDB_THUMBNAIL_FILES
        || !db->pixels 
    #endif
        )
        { 
        musicdb_close( db ); 
        return false;
        }
    
    db->songs_added = 0;
    db->change_counter = 0;

#ifndef MUSICDB_THUMBNAIL_FILES
    preload_pixel_cache( db, pixels_filename );
#endif

    /*
    db->shuffle->header.count = 20;
    srand(11);
    for( int i = 0; i < db->shuffle->header.count; ++i ) {
        bool found = false;
        int index = rand() % db->songs->header.count;
        do {
        bool found = false;
        for( int j = 0; j < i; ++j ) {
            if( index == db->shuffle->items[ j ] ) {
                found = true;
                break;
            }
        }
        if( found ) {
            index = rand() % db->songs->header.count;
        }
        } while( found );
        db->shuffle->items[ i ] = index;
    }
    */

    return true;
    }

uint64_t musicdb_change_counter( music_db_t* db )
    {
    thread_mutex_lock( &db->mutex );
    uint64_t value = db->change_counter;
    thread_mutex_unlock( &db->mutex );
    return value;
    }


bool musicdb_internal_ensure_shuffle_capacity( music_db_t* db )
    {
    if( db->shuffle->header.count >= db->shuffle->header.capacity )
        {
        db->mmap_shuffle = musicdb_internal_mmap_resize( db->mmap_shuffle, 
            sizeof( int ), db->shuffle->header.capacity * 2 );
        if( !db->mmap_shuffle ) return false;
        db->shuffle = (musicdb_shuffle_t*) mmap_data( db->mmap_shuffle );
        if( !db->shuffle ) return false;
        }

    return true;
    }


bool musicdb_internal_ensure_songs_capacity( music_db_t* db )
    {
    if( db->songs->header.count >= db->songs->header.capacity )
        {
        db->mmap_songs = musicdb_internal_mmap_resize( db->mmap_songs, 
            sizeof( musicdb_song_t ), db->songs->header.capacity * 2 );
        if( !db->mmap_songs ) return false;
        db->songs = (musicdb_songs_t*) mmap_data( db->mmap_songs );
        if( !db->songs ) return false;
        }

    return true;
    }


bool musicdb_internal_ensure_artists_capacity( music_db_t* db )
    {
    if( db->artists->header.count >= db->artists->header.capacity )
        {
        db->mmap_artists = musicdb_internal_mmap_resize( db->mmap_artists, 
            sizeof( musicdb_artist_t ), db->artists->header.capacity * 2 );
        if( !db->mmap_artists ) return false;
        db->artists = (musicdb_artists_t*) mmap_data( db->mmap_artists );
        if( !db->artists ) return false;
        }

    return true;
    }


bool musicdb_internal_ensure_albums_capacity( music_db_t* db )
    {
    if( db->albums->header.count >= db->albums->header.capacity )
        {
        db->mmap_albums = musicdb_internal_mmap_resize( db->mmap_albums, 
            sizeof( musicdb_album_t ), db->albums->header.capacity * 2 );
        if( !db->mmap_albums ) return false;
        db->albums = (musicdb_albums_t*) mmap_data( db->mmap_albums );
        if( !db->albums ) return false;
        }

    return true;
    }


bool musicdb_internal_ensure_genres_capacity( music_db_t* db )
    {
    if( db->genres->header.count >= db->genres->header.capacity )
        {
        db->mmap_genres = musicdb_internal_mmap_resize( db->mmap_genres, 
            sizeof( musicdb_genre_t ), db->genres->header.capacity * 2 );
        if( !db->mmap_genres ) return false;
        db->genres = (musicdb_genres_t*) mmap_data( db->mmap_genres );
        if( !db->genres ) return false;
        }

    return true;
    }


bool musicdb_internal_ensure_thumbnails_capacity( music_db_t* db )
    {
    if( db->thumbnails->header.count >= db->thumbnails->header.capacity )
        {
        db->mmap_thumbnails = musicdb_internal_mmap_resize( db->mmap_thumbnails, 
            sizeof( musicdb_thumbnail_t ), db->thumbnails->header.capacity * 2 );
        if( !db->mmap_thumbnails ) return false;
        db->thumbnails = (musicdb_thumbnails_t*) mmap_data( db->mmap_thumbnails );
        if( !db->thumbnails ) return false;
        }

    return true;
    }


#ifndef MUSICDB_THUMBNAIL_FILES
    bool musicdb_internal_ensure_pixels_capacity( music_db_t* db, size_t size )
        {
        if( ( db->pixels->next_free * sizeof( uint32_t ) ) + size > db->pixels->header.size - sizeof( *db->pixels ) )
            {
            size_t new_size = ( db->pixels->next_free * sizeof( uint32_t ) ) + 64 * 1024 * 1024;
            db->mmap_pixels = musicdb_internal_mmap_resize_pixels( db->mmap_pixels, new_size );
            if( !db->mmap_pixels ) return false;
            db->pixels = (musicdb_pixels_t*) mmap_data( db->mmap_pixels );
            if( !db->pixels ) return false;
            }

        return true;
        }
#endif

uint32_t musicdb_internal_add_thumbnail( music_db_t* db, char const* filename, int index, bool* added )
    {
    *added = false;
    uint32_t thumbnail_id = MUSICDB_INVALID_ID;
    for( int j = 0; j < db->thumbnails->header.count; ++j )
        {
        musicdb_thumbnail_t* thumbnail = &db->thumbnails->items[ j ];
        if( strcmp( thumbnail->source_filename, filename ) == 0  && thumbnail->source_image_index == index && thumbnail->last_modified == file_last_changed( filename ) )
            {
            thumbnail_id = thumbnail->id;
            }
        }

    if( thumbnail_id == MUSICDB_INVALID_ID )
        {
        if( !musicdb_internal_ensure_thumbnails_capacity( db ) ) return false;
           
        thumbnail_id = (uint32_t) db->thumbnails->header.count++;
        musicdb_thumbnail_t* thumbnail = &db->thumbnails->items[ thumbnail_id ];
        thumbnail->id = thumbnail_id;
        MUSICDB_INTERNAL_STRNCPY( thumbnail->source_filename, filename );
        thumbnail->source_image_index = index;
        thumbnail->last_modified = file_last_changed( filename );
        thumbnail->initialized = false;
        thumbnail->width = 0;
        thumbnail->height = 0;
        #ifdef MUSICDB_THUMBNAIL_FILES
            MUSICDB_INTERNAL_STRNCPY( thumbnail->thumbs_file, "" );
        #else
            thumbnail->pixels_offset = 0;
        #endif
        *added = true;
        }

    return thumbnail_id;
    }


void trim( char* str )
    {
    char* ptr = str;
    while( *ptr && isspace( (uint8_t)*ptr ) ) ++ptr;
    memmove( str, ptr, strlen( ptr ) );
    ptr = str + strlen( str ) - 1;
    while( ptr > str && isspace( (uint8_t)*ptr ) ) --ptr;
    ptr[ 1 ] = '\0';
    }


static int calculate_mip_size( int w, int h )
    {
    int size = 0;
    while( w > 0 && h > 0 )
        {
        size += w * h;
        w /= 2;
        h /= 2;
        }
    return size;
    }


static uint32_t* generate_thumbnail_from_image_file( const char* filename, size_t* out_size, int* out_width, int* out_height )
    {
    struct stat s;
    stat( filename, &s );
    mmap_t* mmap = mmap_open_read_only( filename, (size_t) s.st_size );
    if( !mmap ) return 0; // TODO: handle thumbnail error               
    int w, h, c;
    stbi_uc* img = stbi_load_from_memory( (stbi_uc*) mmap_data( mmap ), (int) mmap_size( mmap ), &w, &h, &c, 4 );
    mmap_close( mmap );
    if( !img ) return 0; // TODO: handle thumbnail error               

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

    int mip_width = global_max_w;
    int mip_height = global_max_h;
    if( w > h ) mip_height = ( h * mip_height ) / w;
    else if( h > w ) mip_width = ( w * mip_width ) / h;
    mip_width = (int) pow2_ceil( (unsigned int) mip_width );
    mip_height = (int) pow2_ceil( (unsigned int) mip_height );

    size_t newimg_size = sizeof( uint32_t ) * ( 2 + calculate_mip_size( mip_width, mip_height ) );
    uint32_t* newimg = (uint32_t*) malloc( newimg_size );
    uint32_t* ptr = newimg;
    *ptr++ = (uint32_t) mip_width;
    *ptr++ = (uint32_t) mip_height;
    stbir_resize_uint8( img, w, h, 0, (unsigned char*) ptr, mip_width, mip_height, 0, 4 );
    ptr += mip_width * mip_height;
    int mw = mip_width / 2;
    int mh = mip_height / 2;
    while( mw > 0 && mh > 0 )
        {
        stbir_resize_uint8( (unsigned char*)( newimg + 2 ), mip_width, mip_height, 0, (unsigned char*) ptr, mw, mh, 0, 4 );
        ptr += mw * mh;
        mw /= 2;
        mh /= 2;
        }
    stbi_image_free( img );

    *out_size = newimg_size;
    *out_width = w;
    *out_height = h;
    return newimg;
    }


static uint32_t* generate_thumbnail_from_mp3_file( const char* filename, int index, size_t* out_size, int* out_width, int* out_height )
    {
    size_t mp3size = file_size( filename );
    mmap_t* mmap= mmap_open_read_only( filename, mp3size );
    if( !mmap ) return 0;

    size_t size = id3tag_size( mmap_data( mmap ) );
    if( size <= 0 ) {  mmap_close( mmap );  return 0; /* TODO: handle thumbnail error */ }

    id3tag_t* tag = id3tag_load( mmap_data( mmap ), size, ID3TAG_FIELD_PICS, 0 );
    if( !tag ) {  mmap_close( mmap );  return 0; /* TODO: handle thumbnail error */ }

    void const* img_data = tag->pics[ index >= 0 ? index : 0 ].data;
    size_t img_size = tag->pics[ index >= 0 ? index : 0 ].size;
    int w, h, c;
    stbi_uc* img = stbi_load_from_memory( (stbi_uc const*) img_data, (int) img_size, &w, &h, &c, 4 );
    id3tag_free( tag );
    mmap_close( mmap ); 
    if( !img ) return 0; // TODO: handle thumbnail error

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

    int mip_width = global_max_w;
    int mip_height = global_max_h;
    if( w > h ) mip_height = ( h * mip_height ) / w;
    else if( h > w ) mip_width = ( w * mip_width ) / h;
    mip_width = (int) pow2_ceil( (unsigned int) mip_width );
    mip_height = (int) pow2_ceil( (unsigned int) mip_height );

    size_t newimg_size = sizeof( uint32_t ) * ( 2 + calculate_mip_size( mip_width, mip_height ) );
    uint32_t* newimg = (uint32_t*) malloc( newimg_size );
    uint32_t* ptr = newimg;
    *ptr++ = (uint32_t) mip_width;
    *ptr++ = (uint32_t) mip_height;
    stbir_resize_uint8( img, w, h, 0, (unsigned char*) ptr, mip_width, mip_height, 0, 4 );
    ptr += mip_width * mip_height;
    int mw = mip_width / 2;
    int mh = mip_height / 2;
    while( mw > 0 && mh > 0 )
        {
        stbir_resize_uint8( (unsigned char*)( newimg + 2 ), mip_width, mip_height, 0, (unsigned char*) ptr, mw, mh, 0, 4 );
        ptr += mw * mh;
        mw /= 2;
        mh /= 2;
        }
    stbi_image_free( img );

    *out_size = newimg_size;
    *out_width = w;
    *out_height = h;
    return newimg;
    }


char const* remap_genre( char const* genre )
    {
     for( int i = 0; i < sizeof( genres_mappings ) / sizeof( *genres_mappings ); ++i )
        {
        if( genres_mappings[ i ].original == NULL || genres_mappings[ i ].remapped == NULL ) break;
        if( stricmp( genre, genres_mappings[ i ].original) == 0 )
            return genres_mappings[ i ].remapped;
        }

    return genre;
    }


static char const* find_image_in_songs( music_db_t* db, uint32_t album_id, int* out_index )
    {
    char const* found_filename = 0;
    *out_index = -1;
    for( int i = 0; i < db->songs->header.count; ++i )
        {
        musicdb_song_t* song = &db->songs->items[ i ];
        if( song->album_id != album_id ) continue;

        size_t mp3size = file_size( song->filename );
        mmap_t* mmap= mmap_open_read_only( song->filename, mp3size );
        if( !mmap ) continue; 

        size_t size = id3tag_size( mmap_data( mmap ) );
        if( size <= 0 ) 
            { 
            mmap_close( mmap ); 
            continue; 
            }

        id3tag_t* tag = id3tag_load( mmap_data( mmap ), size, ID3TAG_FIELD_PICS, 0 );
        mmap_close( mmap ); 
        if( !tag ) continue; 
    
        for( int j = 0; j < tag->pics_count; ++j )
            {
            bool format_known = false;
            if( strcmp( tag->pics[ j ].mime_type, "image/jpg" ) == 0 ) format_known = true;
            else if( strcmp( tag->pics[ j ].mime_type, "image/jpeg" ) == 0 ) format_known = true;
            else if( strcmp( tag->pics[ j ].mime_type, "image/png" ) == 0 ) format_known = true;
            else if( strcmp( tag->pics[ j ].mime_type, "image/gif" ) == 0 ) format_known = true;
            else if( strcmp( tag->pics[ j ].mime_type, "image/bmp" ) == 0 ) format_known = true;
            else 
                {
                unsigned char jpeg[] = { 0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46 };
                unsigned char* data = (unsigned char*) tag->pics[ j ].data;
                if( memcmp( jpeg, data, sizeof( jpeg ) ) == 0 ) format_known = true;
                }
    
            if( format_known )
                {
                if( tag->pics[ j ].pic_type == ID3TAG_PIC_TYPE_COVER_FRONT )
                    {
                    id3tag_free( tag );   
                    *out_index = j;
                    return song->filename;
                    }
                else if( found_filename == 0 )
                    {
                    found_filename = song->filename;
                    *out_index = j;
                    }
                }
            }
        id3tag_free( tag );
        }
    
    return found_filename;
    }


static void musicdb_internal_remove_song( music_db_t* db, uint32_t song_id );
static uint32_t musicdb_internal_add_album( music_db_t* db, char const* album_title, char const* album_artist, char const* sort_album_artist, char const* path, bool* added );
static uint32_t musicdb_internal_add_artist( music_db_t* db, char const* artist_name, char const* sort_name );
static uint32_t musicdb_internal_add_genre( music_db_t* db, char const* genre_label );

int const TASK_PRIORITY_WRITE_SHUFFLE_LIST = 150;
int const TASK_PRIORITY_CHECK_THUMBNAILS_AT_START = 140;
int const TASK_PRIORITY_GENERATE_THUMBNAIL_GENRE = 130;
int const TASK_PRIORITY_FORCED_ADD_ALBUMS_ARTISTS = 120;
int const TASK_PRIORITY_CREATE_ALBUM_THUMBNAIL = 110;
int const TASK_PRIORITY_GENERATE_THUMBNAIL = 100;
int const TASK_PRIORITY_CREATE_ARTIST_THUMBNAIL = 90;
int const TASK_PRIORITY_ADD_SONG = 80;
int const TASK_PRIORITY_SCAN_DIR = 75;
int const TASK_PRIORITY_ADD_ALBUMS_ARTISTS = 70;
int const TASK_PRIORITY_SORT = 60;
int const TASK_PRIORITY_VERIFY_EXISTING = 40;
int const TASK_PRIORITY_CHECK_THUMBNAILS = 30;
int const TASK_PRIORITY_SCAN_PATH = 20;
int const TASK_PRIORITY_SCAN_PATH_FINALIZE = 15;
int const TASK_PRIORITY_BUILD_DB_INITIAL = 10;
int const TASK_PRIORITY_LOAD_SHUFFLE_LIST = 7;
int const TASK_PRIORITY_CALCULATE_SONG_LENGTH = 5;
int const TASK_PRIORITY_BUILD_DB = 1;

static void cancel_task( void* user_data )
    {
    free( user_data );
    }


typedef struct task_write_shuffle_list_t
    {
    music_db_t* db;
    } task_write_shuffle_list_t;

static void task_write_shuffle_list( void* user_data ) 
    {
    task_write_shuffle_list_t task = *(task_write_shuffle_list_t*) user_data;
    free( user_data );

    thread_mutex_lock( &task.db->mutex ); 
    FILE* fp = fopen( "shuffle.cfg", "w" );
    if( fp ) {
        for( int i = 0; i < task.db->shuffle->header.count; ++i ) {
            int index = task.db->shuffle->items[ i ];
            if( index >= 0 && index < task.db->songs->header.count && task.db->songs->items[ index ].id != MUSICDB_INVALID_ID ) {
                fprintf( fp, "%s\\%s\\%s\n", task.db->songs->items[ index ].artist, task.db->songs->items[ index ].album, task.db->songs->items[ index ].title );
            }
        }
        fclose( fp );
    }
    thread_mutex_unlock( &task.db->mutex ); 
    }


typedef struct task_load_shuffle_list_t
    {
    music_db_t* db;
    } task_load_shuffle_list_t;

static void task_load_shuffle_list( void* user_data ) 
    {
    task_load_shuffle_list_t task = *(task_load_shuffle_list_t*) user_data;
    free( user_data );

    thread_mutex_lock( &task.db->mutex ); 
    task.db->shuffle->header.count = 0;
    FILE* fp = fopen( "shuffle.cfg", "r" );
    if( fp ) {
        while( !feof( fp ) ) 
        {
            char artist[ 128 ];
            char album[ 128 ];
            char title[ 128 ];
            fscanf( fp, "%[^\\]\\%[^\\]\\%[^\n]\n", artist, album, title );
            for( int i = 0; i < task.db->songs->header.count; ++i ) 
                {
                musicdb_song_t* song = &task.db->songs->items[ i ];
                if( song->id != MUSICDB_INVALID_ID && strcmp( song->artist, artist ) == 0 && strcmp( song->album, album ) == 0 && strcmp( song->title, title ) == 0 ) 
                    {
                    musicdb_internal_ensure_shuffle_capacity( task.db );
                    task.db->shuffle->items[ task.db->shuffle->header.count++ ] = i;
                    break;
                    }
                }
        }
        fclose( fp );
    }
    thread_mutex_unlock( &task.db->mutex ); 
    g_shuffle_list_loaded = true;
    }


typedef struct task_generate_thumbnail_t
    {
    music_db_t* db;
    uint32_t id;
    } task_generate_thumbnail_t;

static void task_generate_thumbnail( void* user_data )
    {
    task_generate_thumbnail_t task = *(task_generate_thumbnail_t*) user_data;
    free( user_data );

    if( task.id != MUSICDB_INVALID_ID )
        {
        change_current_task( "Generating thumbnails" );
        thread_mutex_lock( &task.db->mutex );

        musicdb_thumbnail_t thumbnail = task.db->thumbnails->items[ task.id ];
            
        thread_mutex_unlock( &task.db->mutex );

        size_t img_size = 0;
        int w = 0;
        int h = 0;
        uint32_t* img = 0;
        if( stricmp( extname( thumbnail.source_filename ), ".mp3") == 0 )
            img = generate_thumbnail_from_mp3_file( thumbnail.source_filename, thumbnail.source_image_index, &img_size, &w, &h );
        else
            img = generate_thumbnail_from_image_file( thumbnail.source_filename, &img_size, &w, &h );

        #ifdef MUSICDB_THUMBNAIL_FILES
            char thumb_filename[ 260 ];
            create_path( ".cache/thumbs" );
            generate_temp_filename( ".cache/thumbs", "pic", thumb_filename, sizeof( thumb_filename ) );    
            file_save_data( img, img_size, thumb_filename, FILE_MODE_BINARY );
        #endif

        thread_mutex_lock( &task.db->mutex );
        task.db->thumbnails->items[ task.id ].initialized = true;
        task.db->thumbnails->items[ task.id ].width = w;
        task.db->thumbnails->items[ task.id ].height = h;
        #ifdef MUSICDB_THUMBNAIL_FILES
            MUSICDB_INTERNAL_STRNCPY( task.db->thumbnails->items[ task.id ].thumbs_file, thumb_filename );
        #else
            musicdb_internal_ensure_pixels_capacity( task.db, img_size );
            task.db->thumbnails->items[ task.id ].pixels_offset = task.db->pixels->next_free;
            memcpy( task.db->pixels->pixel_data + task.db->pixels->next_free, img, img_size );
            task.db->pixels->next_free += ( img_size + (sizeof( uint32_t ) - 1) ) / sizeof( uint32_t );
        #endif    

        free( img );
        ++task.db->change_counter;                
        thread_mutex_unlock( &task.db->mutex );
        }
    }
    

typedef struct task_create_artist_thumbnail_t
    {
    music_db_t* db;
    uint32_t id;
    int album_count;
    } task_create_artist_thumbnail_t;

static void task_create_artist_thumbnail( void* user_data )
    {
    task_create_artist_thumbnail_t task = *(task_create_artist_thumbnail_t*) user_data;
    free( user_data );

    thread_mutex_lock( &task.db->mutex );
    if( task.id >= (uint32_t) task.db->artists->header.count ) 
        {
        thread_mutex_unlock( &task.db->mutex );
        return;
        }

    musicdb_artist_t* artist = &task.db->artists->items[ task.id ];
    if( artist->id == MUSICDB_INVALID_ID || !( artist->thumb_id == MUSICDB_INVALID_ID || artist->thumb_album_count < task.album_count ) || task.album_count == 0 )
        {
        thread_mutex_unlock( &task.db->mutex );
        return;
        }

    change_current_task( "Generating thumbnails" );

    int images_count = 0;
    img_t* images[ 64 ];
    artist->thumb_album_count = task.album_count;
    for( int i = 0; i < task.db->albums->header.count; ++i )
        {
        if( images_count >= 64 ) break;
        musicdb_album_t* album = &task.db->albums->items[ i ];
        if( album->artist_id != artist->id ) continue;

        if( album->thumb_id == MUSICDB_INVALID_ID || (int)album->thumb_id < 0 || (int)album->thumb_id > task.db->thumbnails->header.count ) continue;

        musicdb_thumbnail_t thumb = task.db->thumbnails->items[ album->thumb_id ];
        if( !thumb.initialized ) continue;

        #ifdef MUSICDB_THUMBNAIL_FILES
            mmap_t* mmap = mmap_open_read_only( thumb.thumbs_file, file_size( thumb.thumbs_file ) );
            if( !mmap ) continue;;
            if( mmap_size( mmap ) < sizeof( uint32_t) * 2 ) { mmap_close( mmap ); continue; }

            uint32_t* ptr = (uint32_t*) mmap_data( mmap );
            if( !ptr ) { mmap_close( mmap ); continue; }
        #else
            uint32_t* ptr = task.db->pixels->pixel_data + thumb.pixels_offset;
        #endif

        int mip_width = (int)*ptr++;
        int mip_height = (int)*ptr++;

        #ifdef MUSICDB_THUMBNAIL_FILES
        if( mmap_size( mmap ) < sizeof( uint32_t ) * ( 2 + calculate_mip_size( mip_width, mip_height ) ) ) { mmap_close( mmap ); continue; }
        #endif

        img_t* img = (img_t*) malloc( sizeof( img_t ) );
        #ifdef MUSICDB_THUMBNAIL_FILES
            img->pixels = (uint32_t*)malloc( mip_width * mip_height * sizeof( uint32_t ) );
            memcpy( img->pixels, ptr, mip_width * mip_height * sizeof( uint32_t ) );
        #else
            img->pixels = ptr;
        #endif
        img->width = mip_width;
        img->height = mip_height;
        #ifdef MUSICDB_THUMBNAIL_FILES
        mmap_close( mmap );
        #endif

        images[ images_count++ ] = img;
        }
    thread_mutex_unlock( &task.db->mutex );

    if( images_count > 0 )
        {
        int out_width = global_max_w;
        int out_height = global_max_h;
        uint32_t* out = (uint32_t*) malloc( sizeof( uint32_t ) * out_width * out_height );
        generate_artist_thumbnail( images_count, images, g_shadow, out, out_width, out_height );

        #ifdef MUSICDB_THUMBNAIL_FILES
        for( int j = 0; j < images_count; ++j ) img_destroy( images[ j ] );
        #endif

        int mip_width = global_max_w;
        int mip_height = global_max_h;

        size_t newimg_size = sizeof( uint32_t ) * ( 2 + calculate_mip_size( mip_width, mip_height ) );
        #ifdef MUSICDB_THUMBNAIL_FILES
        uint32_t* newimg = (uint32_t*) malloc( newimg_size );
        #else
        musicdb_internal_ensure_pixels_capacity( task.db, newimg_size );
        uint32_t* newimg = task.db->pixels->pixel_data + task.db->pixels->next_free;
        #endif
        uint32_t* ptr = newimg;
        *ptr++ = (uint32_t) mip_width;
        *ptr++ = (uint32_t) mip_height;
        memcpy( ptr, out, mip_width * mip_height * sizeof( uint32_t ) );
        ptr += mip_width * mip_height;
        int mw = mip_width / 2;
        int mh = mip_height / 2;
        while( mw > 0 && mh > 0 )
            {
            stbir_resize_uint8( (unsigned char*) out, out_width, out_height, 0, (unsigned char*) ptr, mw, mh, 0, 4 );
            ptr += mw * mh;
            mw /= 2;
            mh /= 2;
            }
        free( out );

        #ifdef MUSICDB_THUMBNAIL_FILES
        char thumb_filename[ 260 ] = "";
        create_path( ".cache/thumbs" );
        generate_temp_filename( ".cache/thumbs", "pic", thumb_filename, sizeof( thumb_filename ) );

        file_save_data( newimg, newimg_size, thumb_filename, FILE_MODE_BINARY );
        #endif

        thread_mutex_lock( &task.db->mutex );
        musicdb_internal_ensure_thumbnails_capacity( task.db );
           
        uint32_t thumbnail_id = (uint32_t) task.db->thumbnails->header.count++;
        musicdb_thumbnail_t* thumbnail = &task.db->thumbnails->items[ thumbnail_id ];
        thumbnail->id = thumbnail_id;
        MUSICDB_INTERNAL_STRNCPY( thumbnail->source_filename, "" );
        thumbnail->source_image_index = -1;
        thumbnail->initialized = true;
        thumbnail->width = out_width;
        thumbnail->height = out_height;
        #ifdef MUSICDB_THUMBNAIL_FILES
            MUSICDB_INTERNAL_STRNCPY( thumbnail->thumbs_file, thumb_filename );
            free( newimg );
        #else
            thumbnail->pixels_offset = task.db->pixels->next_free;
            task.db->pixels->next_free += ( newimg_size + (sizeof( uint32_t ) - 1) ) / sizeof( uint32_t );
        #endif

        task.db->artists->items[ task.id ].thumb_id = thumbnail_id;

        ++task.db->change_counter;
        thread_mutex_unlock( &task.db->mutex );
        }
    }        

    
typedef struct task_create_album_thumbnail_t
    {
    music_db_t* db;
    uint32_t id;
    } task_create_album_thumbnail_t;

static void task_create_album_thumbnail( void* user_data )
    {
    task_create_album_thumbnail_t task = *(task_create_album_thumbnail_t*) user_data;
    free( user_data );

    thread_mutex_lock( &task.db->mutex );
    if( task.id < (uint32_t) task.db->albums->header.count )
        {
        change_current_task( "Generating thumbnails" );
        musicdb_album_t* album = &task.db->albums->items[ task.id ];
        if( album->thumb_id == MUSICDB_INVALID_ID )
            {
            int index = -1;
            char const* filename = find_image_in_songs( task.db, album->id, &index );    
            bool added = false;
            album->thumb_id = MUSICDB_INVALID_ID;
			if( filename )
				{
				album->thumb_id = musicdb_internal_add_thumbnail( task.db, filename, index, &added );
				if( added )
					{
					task_generate_thumbnail_t* thumb_task = (task_generate_thumbnail_t*) malloc( sizeof( *thumb_task ) );
					thumb_task->db = task.db;
					thumb_task->id = album->thumb_id;
					background_tasks_add( task.db->background_tasks, TASK_PRIORITY_GENERATE_THUMBNAIL, task_generate_thumbnail, cancel_task, thumb_task );
					}
				}
            }
        }
    ++task.db->change_counter;
    thread_mutex_unlock( &task.db->mutex );       
    }


typedef struct task_check_thumbs_t
    {
    music_db_t* db;
    } task_check_thumbs_t;

static void task_check_thumbs( void* user_data )
    {
    task_check_thumbs_t task = *(task_check_thumbs_t*) user_data;
    free( user_data );

    thread_mutex_lock( &task.db->mutex );
    for( int i = 0; i < task.db->thumbnails->header.count; ++i )
        {
        musicdb_thumbnail_t* thumb = &task.db->thumbnails->items[ i ];
        if( !thumb->initialized )
            {
            printf( "thumb: %s\n", thumb->source_filename );
            task_generate_thumbnail_t* thumb_task = (task_generate_thumbnail_t*) malloc( sizeof( *thumb_task ) );
            thumb_task->db = task.db;
            thumb_task->id = thumb->id;
            background_tasks_add( task.db->background_tasks, TASK_PRIORITY_GENERATE_THUMBNAIL, task_generate_thumbnail, cancel_task, thumb_task );
            }
        }
    thread_mutex_unlock( &task.db->mutex );       
    }


typedef struct task_add_albums_artists_t
    {
    music_db_t* db;
    } task_add_albums_artists_t;

static void task_add_albums_artists( void* user_data )
    {
    task_add_albums_artists_t task = *(task_add_albums_artists_t*) user_data;
    free( user_data );

    thread_mutex_lock( &task.db->mutex );

    if( task.db->songs_added == 0 )
        {
        thread_mutex_unlock( &task.db->mutex );
        return;
        }
        
    change_current_task( "Adding albums/artists" );

    for( int i = 0; i < task.db->albums->header.count; ++i )
        {
        musicdb_album_t* album = &task.db->albums->items[ i ];
        album->track_count = 0;
        }
    

    for( int i = 0; i < task.db->songs->header.count; ++i )
        {
        musicdb_song_t* song = &task.db->songs->items[ i ];
        if( song->id == MUSICDB_INVALID_ID ) continue;
        bool added = false;
        song->album_id = musicdb_internal_add_album( task.db, song->album, song->album_artist, song->sort_album_artist, dirname( song->filename ), &added );       

        char const* artist = song->album_artist;
        if( *artist == '\0' ) artist = song->artist;
        musicdb_album_t* album = &task.db->albums->items[ song->album_id ];
        album->artist_id = musicdb_internal_add_artist( task.db, artist, song->sort_album_artist );
        MUSICDB_INTERNAL_STRNCPY( album->artist, artist );
        album->compilation = album->compilation || song->compilation;
        album->year = song->year;
        album->various = album->various || ( stricmp( artist, "Various Artists" ) == 0 );
        album->track_count++;
        if( album->thumb_id == MUSICDB_INVALID_ID && added)
            {
            task_create_album_thumbnail_t* thumb_task = (task_create_album_thumbnail_t*) malloc( sizeof( *thumb_task ) );
            thumb_task->db = task.db;
            thumb_task->id = album->id;
            background_tasks_add( task.db->background_tasks, TASK_PRIORITY_CREATE_ALBUM_THUMBNAIL, task_create_album_thumbnail, cancel_task, thumb_task );
            }
        
        if( *song->genre )
            {
            uint32_t genre_id = musicdb_internal_add_genre( task.db, song->genre );
            bool found = false;
            for( int j = 0; j < album->genres_count; ++j )
                {
                if( album->genres_id[ j ] == genre_id )
                    {
                    found = true;
                    break;
                    }
                }
            if( !found )
                {
                if( album->genres_count >= sizeof( album->genres_id ) / sizeof( *album->genres_id ) )
                    continue;
                album->genres_id[ album->genres_count++ ] = genre_id;
                }
            }
        }

    for( int j = 0; j < task.db->albums->header.count; ++j )
        {
        musicdb_album_t* album = &task.db->albums->items[ j ];
        bool is_various = false;
        char longest_common[ 256 ] = "";
        bool multiple_years = false;
        for( int i = 0; i < task.db->songs->header.count; ++i )
            {
            musicdb_song_t* song = &task.db->songs->items[ i ];
            if( song->album_id == album->id )
                {
                if( song->year && song->year != album->year ) 
                    {
                    multiple_years = true;
                    }
                char const* artist = song->album_artist;
                if( *artist == '\0' ) artist = song->artist;
                if( strcmp( artist, album->artist ) != 0 )
                    {
                    int len = 0;
                    char const* str_a = artist;
                    char const* str_b = album->artist;
                    while( *str_a == *str_b && *str_a )
                        {
                        ++len;
                        ++str_a;
                        ++str_b;
                        }
                    if( *longest_common == 0 || len < (int) strlen( longest_common ) )
                        {
                        strncpy( longest_common, artist, (size_t) len );
                        longest_common[ len ] = '\0';
                        }
                    is_various = true;
                    }
                }

            }
        if( multiple_years ) 
            {
            album->year = 0;
            }
        if( is_various )
            {
            if( strlen( longest_common ) > 3 )
                {
                uint32_t new_artist = musicdb_internal_add_artist( task.db, longest_common, longest_common );
                strcpy( album->artist, longest_common );
                album->artist_id = new_artist;
                album->various = false;
            }
            else
                {
                uint32_t various_artists = musicdb_internal_add_artist(task.db,"Various Artists","Various Artists");
                strcpy(album->artist,"Various Artists");
                album->artist_id = various_artists;
                album->various = true;
                }
            }
        }

    for( int i = 0; i < task.db->artists->header.count; ++i )
        {
        musicdb_artist_t* artist = &task.db->artists->items[ i ];
        if( artist->id == MUSICDB_INVALID_ID ) continue;
        int prev_count = artist->album_count;
        artist->album_count = 0;
        for( int j = 0; j < task.db->albums->header.count; ++j )
            {
            musicdb_album_t* album = &task.db->albums->items[ j ];
            if( album->artist_id == artist->id )
                {
                artist->album_count++;
                }
            }
        if( artist->album_count != prev_count && stricmp( artist->name, "Various Artists" ) != 0 )
            {
            artist->thumb_id = MUSICDB_INVALID_ID;
            task_create_artist_thumbnail_t* thumb_task = (task_create_artist_thumbnail_t*) malloc( sizeof( *thumb_task ) );
            thumb_task->db = task.db;
            thumb_task->id = artist->id;
            thumb_task->album_count = artist->album_count;
            background_tasks_add( task.db->background_tasks, TASK_PRIORITY_CREATE_ARTIST_THUMBNAIL, task_create_artist_thumbnail, cancel_task, thumb_task );
            }

        }

    for( int i = 0; i < task.db->genres->header.count; ++i )
        {
        musicdb_genre_t* genre = &task.db->genres->items[ i ];
        if( genre->id == MUSICDB_INVALID_ID ) continue;
        genre->album_count = 0;
        for( int j = 0; j < task.db->albums->header.count; ++j )
            {
            musicdb_album_t* album = &task.db->albums->items[ j ];
            for( int k = 0; k < album->genres_count; ++k )
                {
                if( album->genres_id[ k ] == genre->id )
                    {
                    genre->album_count++;
                    }
                }
            }
        }

    task.db->songs_added = 0;
    ++task.db->change_counter;

    thread_mutex_unlock( &task.db->mutex );
    }


typedef struct calculate_mp3_length_t
    {
    float length;
    float acc;
    } calculate_mp3_length_t;


int calculate_mp3_length( void *user_data, const uint8_t *frame, int frame_size, size_t offset, mp3dec_frame_info_t *info )
    {
    (void) frame, frame_size, offset;
    if( !info->bitrate_kbps ) return 1;
    calculate_mp3_length_t* ctx = (calculate_mp3_length_t*) user_data;
    float len = ((8.0f*info->frame_bytes)/(1000.0f*info->bitrate_kbps));
    ctx->length += len;
    ctx->acc += len;
    if( ctx->acc > 10 ) {
        ctx->acc = 0;
        sleep(5);
    }
    return 0;
    }


typedef struct task_calculate_song_length_t
    {
    music_db_t* db;
    int song_id;
    } task_calculate_song_length_t; 


static void task_calculate_song_length( void* user_data )
    {
    task_calculate_song_length_t task = *(task_calculate_song_length_t*) user_data;
    free( user_data );

    if( task.db->songs->items[ task.song_id ].id != MUSICDB_INVALID_ID && !task.db->songs->items[ task.song_id ].length_in_seconds )
        {
        change_current_task( "Calculating song lengths" );
        char const* filename = task.db->songs->items[ task.song_id ].filename;
        size_t mp3size = task.db->songs->items[ task.song_id ].filesize;
        mmap_t* mmap= mmap_open_read_only( filename, mp3size );
        if( mmap ) 
            {
            calculate_mp3_length_t ctx = { 0.0f };
            mp3dec_iterate_buf( mmap_data( mmap ), mmap_size( mmap), calculate_mp3_length, &ctx );
            thread_mutex_lock( &task.db->mutex );
            task.db->songs->items[ task.song_id ].length_in_seconds = (int)( ctx.length + 0.5f );
            ++task.db->change_counter;
            thread_mutex_unlock( &task.db->mutex );
            mmap_close( mmap ); 
            }
        }
    }


typedef struct estimate_mp3_length_t {
    float length;
    size_t file_size;
} estimate_mp3_length_t;

int estimate_mp3_length( void *user_data, const uint8_t *frame, int frame_size, size_t offset, mp3dec_frame_info_t *info ) {
    #define MP3_ENDIAN_SWAP( x ) ( (((((x)) & 0xFF000000) >> 24) | ((((x)) & 0x00FF0000) >> 8) | ((((x)) & 0x0000FF00) << 8) | ((((x)) & 0x000000FF) << 24) ) )
    
    (void) frame, frame_size, offset;
    estimate_mp3_length_t* ctx = (estimate_mp3_length_t*) user_data;
    float frame_length = ( ( 8.0f * info->frame_bytes ) / ( 1000.0f * info->bitrate_kbps ) );

    int header_offset = 21;
    header_offset = ( info->layer == 3 && info->channels == 2 ) ? 36 : header_offset;
    header_offset = ( info->layer != 3 && info->channels == 1 ) ? 13 : header_offset;
	char const* header_id = (char const*)( frame + header_offset );
	header_offset += 4;
    if( strnicmp( header_id, "Xing", 4 ) == 0 || strnicmp( header_id, "Info", 4 ) == 0 ) {
	    uint32_t flags = *(uint32_t*)( frame + header_offset ); 
    	header_offset += 4;
        flags = MP3_ENDIAN_SWAP( flags );
        if( flags & 0x1 ) {
            int frames_count = *(uint32_t*)( frame + header_offset );
            frames_count = MP3_ENDIAN_SWAP( frames_count );
            if( frames_count ) {
                ctx->length = frame_length * frames_count;
            }
        }
        return 1;
    }

    header_offset = 36;
	header_id = (char const*)( frame + header_offset );
    if( strnicmp( header_id, "VBRI", 4 ) == 0 ) {
        header_offset += 14;
		int frames_count = *(uint32_t*)( frame + header_offset );
        frames_count = MP3_ENDIAN_SWAP( frames_count );
        if( frames_count ) {
            ctx->length = frame_length * frames_count;
        }
        return 1;
    } 

    ctx->length = ( ( ctx->file_size - offset ) * 8.0f ) / ( info->bitrate_kbps * 1000.0f );
    return 1;

    #undef MP3_ENDIAN_SWAP
    }


typedef struct task_add_song_t
    {
    music_db_t* db;
    char file[ 260 ];
    } task_add_song_t; 

static void task_add_song( void* user_data )
    {
    task_add_song_t task = *(task_add_song_t*) user_data;
    free( user_data );

    change_current_task( "Adding songs" );

    size_t mp3size = file_size( task.file );
    mmap_t* mmap= mmap_open_read_only( task.file, mp3size );
    if( !mmap ) return;

    size_t size = id3tag_size( mmap_data( mmap ) );

    id3tag_t* tag = id3tag_load( mmap_data( mmap ), size, 
        ID3TAG_FIELD_TITLE | ID3TAG_FIELD_ARTIST | ID3TAG_FIELD_ALBUM_ARTIST | ID3TAG_FIELD_SORT_ALBUM_ARTIST | 
        ID3TAG_FIELD_SORT_ARTIST | ID3TAG_FIELD_ALBUM | ID3TAG_FIELD_GENRE | ID3TAG_FIELD_YEAR | 
        ID3TAG_FIELD_TRACK | ID3TAG_FIELD_DISC | ID3TAG_FIELD_COMPILATION | ID3TAG_FIELD_TRACK_LENGTH |
        ID3TAG_FIELD_REPLAYGAIN_TRACK_GAIN | ID3TAG_FIELD_REPLAYGAIN_ALBUM_GAIN
        , 0 );
    
    id3tag_t* tagv1 = id3tag_load_id3v1( (void*)( ( (uintptr_t) mmap_data( mmap ) ) + mp3size - 128 ), 128, 0 );

    char const* title = "";
    char const* artist = "";
    char const* sort_album_artist = "";
    char const* sort_artist = "";
    char const* album_artist = "";
    char const* album = "";
    char const* genre = "";
    int year = 0;
    int track = 0;
    int disc = 0;
    int length_in_seconds = 0;
    float replaygain_track_gain = 1.0f;
    float replaygain_album_gain = 1.0f;

    if( tag )
        {
        if( tag->title ) title = tag->title;        
        if( tag->artist && *tag->artist ) artist = tag->artist;               
        if( tag->album_artist && *tag->album_artist) album_artist = tag->album_artist;
        if( tag->sort_album_artist && *tag->sort_album_artist ) sort_album_artist = tag->sort_album_artist;
        if( tag->sort_artist && *tag->sort_artist ) sort_artist = tag->sort_artist;
        if( tag->album ) album = tag->album; 
        if( tag->genre ) genre = tag->genre;
        if( tag->year ) year = atoi( tag->year );
        if( tag->track ) track = atoi( tag->track );
        if( tag->disc ) disc = atoi( tag->disc );
        if( tag->track_length ) length_in_seconds = ( tag->track_length + 999 ) / 1000;
        if( tag->replaygain_track_gain ) 
            replaygain_track_gain = powf( 10.0f, tag->replaygain_track_gain / 20.0f );
        if( tag->replaygain_album_gain ) 
            replaygain_album_gain = powf( 10.0f, tag->replaygain_album_gain / 20.0f );
        }

    if( tagv1 )
        {
        if( ( tag && !*title ) && tagv1->title ) title = tagv1->title;
        if( ( tag && !*artist ) && tagv1->artist  ) artist = tagv1->artist;
        if( ( tag && !*album ) && tagv1->album ) album = tagv1->album;
        if( ( tag && !*genre) && tagv1->genre ) genre = tagv1->genre;
        if( ( tag && !year ) && tagv1->year ) year = atoi( tagv1->year );
        if( ( tag && !track ) && tagv1->track ) track = atoi( tagv1->track );
        if( ( tag && !disc ) && tagv1->disc ) disc = atoi( tagv1->disc );
        }

    char const* v1genres[] = { "Blues", "Classic rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop",
        "Jazz", "Metal", "New Age", "Oldies", "Other", "Pop", "Rhythm and Blues", "Rap", "Reggae", "Rock", "Techno",
        "Industrial", "Alternative", "Ska", "Death metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", 
        "Trip-Hop", "Vocal", "Jazz & Funk", "Fusion", "Trance", "Classical", "Instrumental", "Acid", "House", 
        "Game", "Sound clip", "Gospel", "Noise", "Alternative Rock", "Bass", "Soul", "Punk", "Space", "Meditative",
        "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic",
        "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta", "Top 40", "Christian Rap",
        "Pop/Funk", "Jungle", "Native US", "Cabaret", "New Wave", "Psychedelic", "Rave", "Show tunes", "Trailer",
        "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical", "Rock 'n' Roll", "Hard Rock",
        "Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion", "Bebop", "Latin",
        };

    if( *genre && genre[ 0 ] == '(' )
        {
        bool num = true;
        char const* str = genre + 1;
        while( *str )
            {
            if( *str == ')' ) break;
            if( !isdigit( *str) ) { num = false; break; }
            ++str;
            }
        int len = (int)( str - ( genre + 1 ) );
        if( len > 0 && len <= 2 )
            {
            char buffer[ 16 ];
            strncpy( buffer, genre + 1, (size_t) len );
            buffer[ len ] = '\0';
            int id = atoi( buffer );

            if( id >= 0 && id < sizeof( v1genres ) / sizeof( *v1genres ) ) genre = v1genres[ id ];
            }
        }
    else
        {
        int id = atoi( genre );
        if( id > 0 && id < sizeof( v1genres ) / sizeof( *v1genres ) ) genre = v1genres[ id ];
        }

    if( *title == '\0' )
        {
        title = basename( task.file, extname( task.file ) );
        }

    if( *artist == '\0' )
        {
        artist = "<Unknown Artist>";
        }

    if( *album == '\0' )
        {
        album = "<Unknown Album>";
        }

    if( *genre == '\0' )
        {
        genre = "<Unknown>";
        }

    thread_mutex_lock( &task.db->mutex );

    if( !musicdb_internal_ensure_songs_capacity( task.db ) ) 
        {
        if( tagv1 ) id3tag_free( tagv1 );
        if( tag ) id3tag_free( tag );
        mmap_close( mmap ); 
        thread_mutex_unlock( &task.db->mutex );
        return; 
        }

    musicdb_song_t* song = &task.db->songs->items[ task.db->songs->header.count ];
    song->id = (uint32_t) task.db->songs->header.count++;
    MUSICDB_INTERNAL_STRNCPY( song->title, title );
    trim( song->title );
    MUSICDB_INTERNAL_STRNCPY( song->album, album );
    trim( song->album);
    MUSICDB_INTERNAL_STRNCPY( song->artist, artist );
    trim( song->artist );
    MUSICDB_INTERNAL_STRNCPY( song->album_artist, album_artist );
    trim( song->album_artist );
    if( sort_artist ) MUSICDB_INTERNAL_STRNCPY( song->sort_album_artist, sort_artist );
    if( sort_album_artist ) MUSICDB_INTERNAL_STRNCPY( song->sort_album_artist, sort_album_artist );
    trim( song->sort_album_artist );
    MUSICDB_INTERNAL_STRNCPY( song->genre, genre );
    trim( song->genre );
    song->album_id = MUSICDB_INVALID_ID;
    song->mixtape_id = MUSICDB_INVALID_ID;
    song->length_in_seconds = length_in_seconds > 0 ? length_in_seconds : 0; 
    song->disc_number = disc;
    song->track_number = track;
    song->year = year;
    song->compilation = tag && tag->compilation ? atoi( tag->compilation ) != 0 : false;
    song->track_gain = replaygain_track_gain;
    song->album_gain = replaygain_album_gain;

    MUSICDB_INTERNAL_STRNCPY( song->filename, task.file );
    song->filesize = file_size( song->filename );
    song->last_changed = file_last_changed( song->filename );

    estimate_mp3_length_t ctx = { 0.0f, mp3size };
    mp3dec_iterate_buf( mmap_data( mmap ), mp3size, estimate_mp3_length, &ctx );
    int estimated_length_in_seconds = (int)( ctx.length + 0.5f ); 

    if( estimated_length_in_seconds && ( !song->length_in_seconds || abs( song->length_in_seconds - estimated_length_in_seconds ) > 600 ) )
        {
        song->length_in_seconds = estimated_length_in_seconds;
        }

    // Verify song length estimate (debug purposes only)
    //calculate_mp3_length_t ctx2 = { 0.0f };
    //mp3dec_iterate_buf( mmap_data( mmap ), mmap_size( mmap), calculate_mp3_length, &ctx2 );
    //int calculated_length_in_seconds = (int)( ctx2.length + 0.5f ); 
    //if( abs( estimated_length_in_seconds - calculated_length_in_seconds ) > 5 ) 
    //    printf( "EST: %d  CALC: %d  %s\n", estimated_length_in_seconds, calculated_length_in_seconds, task.file );

    int songs_added = ++task.db->songs_added;
    int total_songs = task.db->songs->header.count;
    thread_mutex_unlock( &task.db->mutex );

    //printf( "Song added: %s\n", task.file );
    // printf( "%s %s [%d] %d %s (%s)\n", artist, album, year, track, title, genre );

    if( tagv1 ) id3tag_free( tagv1 );
    if( tag ) id3tag_free( tag );
    mmap_close( mmap );     

    int limit = total_songs / 2;
    if( limit > 500 ) limit = 500;
    if( songs_added >= limit )
        {
        task_add_albums_artists_t* add_task = (task_add_albums_artists_t*) malloc( sizeof( *add_task ) );
        add_task->db = task.db;
        background_tasks_add( task.db->background_tasks, TASK_PRIORITY_FORCED_ADD_ALBUMS_ARTISTS, task_add_albums_artists, cancel_task, add_task );
        }

    if( !song->length_in_seconds )
        {
        task_calculate_song_length_t* length_task = (task_calculate_song_length_t*) malloc( sizeof( *length_task ) );
        length_task->db = task.db;
        length_task->song_id = song->id;
        background_tasks_add( task.db->background_tasks, TASK_PRIORITY_CALCULATE_SONG_LENGTH, task_calculate_song_length, cancel_task, length_task );
        }
    }


typedef struct task_verify_existing_t
    {
    music_db_t* db;
    uint32_t song_id;
    char file[ 260 ];
    time_t last_changed;
    size_t filesize;
    } task_verify_existing_t;

static void task_verify_existing( void* user_data )
    {
    change_current_task( "Verifying database" );
    task_verify_existing_t task = *(task_verify_existing_t*) user_data;
    free( user_data );

    if( task.song_id == MUSICDB_INVALID_ID ) return;

    size_t filesize = file_size( task.file );
    if( task.filesize != filesize || task.last_changed != file_last_changed( task.file ) )
        {
        thread_mutex_lock( &task.db->mutex );
        printf( "Change detected! %s\n", task.file );
        if( filesize == 0 )
            {
            printf( "File removed! %s\n", task.file );
            musicdb_internal_remove_song( task.db, task.song_id );
            ++task.db->change_counter;
            }
        else
            {
            musicdb_internal_remove_song( task.db, task.song_id );

            task_add_song_t* add_task = (task_add_song_t*) malloc( sizeof( *add_task ) );
            add_task->db = task.db;
            strcpy( add_task->file, task.file );
            background_tasks_add( task.db->background_tasks, TASK_PRIORITY_ADD_SONG, task_add_song, cancel_task, add_task );
                            
            ++task.db->change_counter;
            }
        thread_mutex_unlock( &task.db->mutex );
        }
    else
        {
        thread_mutex_lock( &task.db->mutex );
        if( task.db->songs->items[ task.song_id ].id != MUSICDB_INVALID_ID && !task.db->songs->items[ task.song_id ].length_in_seconds )
            {
            task_calculate_song_length_t* length_task = (task_calculate_song_length_t*) malloc( sizeof( *length_task ) );
            length_task->db = task.db;
            length_task->song_id = task.song_id;
            background_tasks_add( task.db->background_tasks, TASK_PRIORITY_CALCULATE_SONG_LENGTH, task_calculate_song_length, cancel_task, length_task );
            }
        thread_mutex_unlock( &task.db->mutex );
        }
    }


typedef struct task_scan_dir_t
    {
    music_db_t* db;
    char path[ 260 ];
    } task_scan_dir_t;

static void task_scan_dir( void* user_data )
    {
    //change_current_task( "Scanning files" );
    task_scan_dir_t task = *(task_scan_dir_t*) user_data;
    free( user_data );

    dir_t* dir = dir_open( task.path );
    if( !dir ) return;

    dir_entry_t* entry = dir_read( dir );
    while( entry )
        {
        if( dir_is_file( entry ) && stricmp( cextname( dir_name( entry ) ), ".mp3" ) == 0 )
            {
            char filename[ 256 ];
            strcpy( filename, task.path );
            pathcat( filename, dir_name( entry ) );

            bool found = false;
            for( int i = 0; i < task.db->songs->header.count; ++i )
                {
                musicdb_song_t* song = &task.db->songs->items[ i ];
                if( stricmp( song->filename, filename ) == 0 )
                    {
                    //if( song->filesize == file_size( filename ) || song->last_changed == file_last_changed( filename ) )
                        {
                        // Already added and up to date
                        found = true;
                        break;
                        }
                    }
                }
            if( !found )
                {
                task_add_song_t* add_task = (task_add_song_t*) malloc( sizeof( *add_task ) );
                add_task->db = task.db;
                strcpy( add_task->file, filename );
                background_tasks_add( task.db->background_tasks, TASK_PRIORITY_ADD_SONG, task_add_song, cancel_task, add_task );
                }
            }

        if( dir_is_folder( entry ) && strcmp( dir_name( entry ), "." ) != 0 && strcmp( dir_name( entry ), ".." ) != 0 )
            {
            char new_path[ 256 ];
            strcpy( new_path, task.path );
            pathcat( new_path, dir_name( entry ) );

            task_scan_dir_t* scan_task = (task_scan_dir_t*) malloc( sizeof( *scan_task ) );
            scan_task->db = task.db;
            strcpy( scan_task->path, new_path );
            background_tasks_add( task.db->background_tasks, TASK_PRIORITY_SCAN_DIR, task_scan_dir, cancel_task, scan_task );
            }

        entry = dir_read( dir );
        }

    dir_close( dir );       
    }


typedef struct task_scan_path_t
    {
    music_db_t* db;
    char path[ 260 ];
    } task_scan_path_t;

static void task_scan_path( void* user_data )
    {
    change_current_task( "Scan after sync" );

    task_scan_path_t task = *(task_scan_path_t*) user_data;
    free( user_data );

    printf( "Scan path: %s\n", task.path );

    thread_mutex_lock( &task.db->mutex );

    for( int i = 0; i < task.db->songs->header.count; ++i )
        {
        musicdb_song_t* song = &task.db->songs->items[ i ];
        if( stricmp( cdirname( song->filename ), task.path ) == 0 )
            {
            size_t filesize = file_size( song->filename );
            if( song->filesize != filesize || song->last_changed != file_last_changed( song->filename ) )
                {
                printf( "Change detected! %s\n", song->filename );
                if( filesize == 0 )
                    {
                    printf( "File removed! %s\n", song->filename );
                    musicdb_internal_remove_song( task.db, song->id );
                    ++task.db->change_counter;
                    }
                else
                    {
                    printf( "Remove song: %s\n", song->filename );
                    musicdb_internal_remove_song( task.db, song->id );                
                    ++task.db->change_counter;
                    }
                }
            }
        }

    dir_t* dir = dir_open( task.path );
    if( !dir ) return;

    dir_entry_t* entry = dir_read( dir );
    while( entry )
        {
        if( dir_is_file( entry ) && stricmp( cextname( dir_name( entry ) ), ".mp3" ) == 0 )
            {
            char filename[ 256 ];
            strcpy( filename, task.path );
            pathcat( filename, dir_name( entry ) );

            bool found = false;
            for( int i = 0; i < task.db->songs->header.count; ++i )
                {
                musicdb_song_t* song = &task.db->songs->items[ i ];
                if( stricmp( song->filename, filename ) == 0 )
                    {
                    if( song->filesize == file_size( filename ) || song->last_changed == file_last_changed( filename ) )
                        {
                        // Already added and up to date
                        found = true;
                        break;
                        }
                    }
                }
            if( !found )
                {
                task_add_song_t* add_task = (task_add_song_t*) malloc( sizeof( *add_task ) );
                add_task->db = task.db;
                strcpy( add_task->file, filename );
                printf( "Add song: %s\n", filename );
                background_tasks_add( task.db->background_tasks, TASK_PRIORITY_ADD_SONG, task_add_song, cancel_task, add_task );
                }
            }

        entry = dir_read( dir );
        }

    dir_close( dir );              

    thread_mutex_unlock( &task.db->mutex );
    change_current_task( "" );

    }


typedef struct task_build_db_t
    {
    music_db_t* db;
    char path[ 32 ][ 260 ];
    int count;
    int sleep_seconds;
    } task_build_db_t;

static void task_build_db( void* user_data )
    {

    if( ( (task_build_db_t*) user_data )->sleep_seconds > 0 ) 
    {
        task_build_db_t* task = (task_build_db_t*) user_data;
        --task->sleep_seconds;
        if( !task->db->filewatches[ 0 ] ) 
            sleep( 1000 );
        else if( filewatch_multi_check( task->db->filewatches, task->count, 1000 ) )
            task->sleep_seconds = 0;

        background_tasks_add( task->db->background_tasks, TASK_PRIORITY_BUILD_DB, task_build_db, cancel_task, task );
        return;
    }

    change_current_task( "Building database" );

    task_build_db_t task = *(task_build_db_t*) user_data;
    free( user_data );

    thread_mutex_lock( &task.db->mutex );

    for( int i = 0; i < task.db->songs->header.count; ++i )
        {
        musicdb_song_t* song = &task.db->songs->items[ i ];
        if( song->id != MUSICDB_INVALID_ID ) 
            {
            task_verify_existing_t* verify_task = (task_verify_existing_t*) malloc( sizeof( *verify_task ) );
            verify_task->db = task.db;
            verify_task->song_id = song->id;
            strcpy( verify_task->file, song->filename );
            verify_task->filesize = song->filesize;
            verify_task->last_changed = song->last_changed;
            background_tasks_add( task.db->background_tasks, TASK_PRIORITY_VERIFY_EXISTING, task_verify_existing, cancel_task, verify_task );
            }
        }

    for( int i = 0; i < task.db->albums->header.count; ++i )
        {
        musicdb_artist_t* artist = &task.db->artists->items[ i ];
        if( artist->id != MUSICDB_INVALID_ID ) 
            {
            if( artist->thumb_id != MUSICDB_INVALID_ID )
                {
                // TODO: Regenerate if album_count changed
                musicdb_thumbnail_t* thumb = &task.db->thumbnails->items[ artist->thumb_id ];
                if( !thumb->initialized )
                    artist->thumb_id = MUSICDB_INVALID_ID;
                }
            if( artist->thumb_id == MUSICDB_INVALID_ID && stricmp( artist->name, "Various Artists" ) != 0 )
                {
                task_create_artist_thumbnail_t* thumb_task = (task_create_artist_thumbnail_t*) malloc( sizeof( *thumb_task ) );
                thumb_task->db = task.db;
                thumb_task->id = artist->id;
                thumb_task->album_count = artist->album_count;
                background_tasks_add( task.db->background_tasks, TASK_PRIORITY_CREATE_ARTIST_THUMBNAIL, task_create_artist_thumbnail, cancel_task, thumb_task );
                }
            }
        }

    for( int i = 0; i < task.count; ++i )
        {
        task_scan_dir_t* scan_task = (task_scan_dir_t*) malloc( sizeof( *scan_task ) );
        scan_task->db = task.db;
        strcpy( scan_task->path, task.path[ i ] );
        background_tasks_add( task.db->background_tasks, TASK_PRIORITY_SCAN_DIR, task_scan_dir, cancel_task, scan_task );
        }

    task_add_albums_artists_t* add_task = (task_add_albums_artists_t*) malloc( sizeof( *add_task ) );
    add_task->db = task.db;
    background_tasks_add( task.db->background_tasks, TASK_PRIORITY_ADD_ALBUMS_ARTISTS, task_add_albums_artists, cancel_task, add_task );

    task_check_thumbs_t* thumbs_task = (task_check_thumbs_t*) malloc( sizeof( *thumbs_task ) );
    thumbs_task->db = task.db;
    background_tasks_add( task.db->background_tasks, TASK_PRIORITY_CHECK_THUMBNAILS_AT_START, task_check_thumbs, cancel_task, thumbs_task );

    task_check_thumbs_t* thumbs_task_later = (task_check_thumbs_t*) malloc( sizeof( *thumbs_task ) );
    thumbs_task_later->db = task.db;
    background_tasks_add( task.db->background_tasks, TASK_PRIORITY_CHECK_THUMBNAILS, task_check_thumbs, cancel_task, thumbs_task_later );

    task_build_db_t* build_task = (task_build_db_t*) malloc( sizeof( *build_task ) );
    build_task->db = task.db;
    for( int i = 0; i < task.count; ++i )
        {
        strcpy( build_task->path[ i ], task.path[ i ] );
        }
    build_task->count = task.count;

    for( int i = 0; i < sizeof( task.db->filewatches ) / sizeof( *task.db->filewatches ); ++i ) 
        if( task.db->filewatches[ i ] ) filewatch_destroy( task.db->filewatches[ i ] );
    memset( task.db->filewatches, 0, sizeof( task.db->filewatches ) );

    for( int i = 0; i < task.count; ++i ) 
        task.db->filewatches[ i ] = filewatch_create( task.path[ i ] );
    
    build_task->sleep_seconds = 2 * 60 * 60; // do a full scan every couple of hours, for general house keeping (to not rely entirely on filewatch)
    background_tasks_add( task.db->background_tasks, TASK_PRIORITY_BUILD_DB, task_build_db, cancel_task, build_task );
    thread_mutex_unlock( &task.db->mutex );

    change_current_task( "" );
    }


void fix_path( char* str )
    {
    for( char* s = str; *s != '\0'; ++s )
        {
        if( *s == '/' ) 
            *s = '\\';
        }
    }


bool musicdb_load_shuffle_list( music_db_t* db )
    {
    task_load_shuffle_list_t* task = (task_load_shuffle_list_t*) malloc( sizeof( *task ) );
    task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_LOAD_SHUFFLE_LIST, task_load_shuffle_list, cancel_task, task );
    return true;
    }



bool musicdb_build( music_db_t* db, char const** paths, int count )
    {
    task_build_db_t* task = (task_build_db_t*) malloc( sizeof( *task ) );
    task->db = db;
    task->count = count > 32 ? 32 : count;
    for( int i = 0; i < task->count; ++i )
        {
        strcpy( task->path[ i ], paths[ i ] );
        fix_path( task->path[ i ] );
        }
    task->sleep_seconds = 0;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_BUILD_DB_INITIAL, task_build_db, cancel_task, task );
    return true;
    }


bool musicdb_scan_path( music_db_t* db, char const* path )
    {
    task_scan_path_t* task = (task_scan_path_t*) malloc( sizeof( *task ) );
    task->db = db;
    strcpy( task->path, path );
    fix_path( task->path );
    background_tasks_add( db->background_tasks, TASK_PRIORITY_SCAN_PATH, task_scan_path, cancel_task, task );
    return true;
    }

bool musicdb_scan_path_finalize( music_db_t* db )
    {
    task_add_albums_artists_t* add_task = (task_add_albums_artists_t*) malloc( sizeof( *add_task ) );
    add_task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_SCAN_PATH_FINALIZE, task_add_albums_artists, cancel_task, add_task );

    task_check_thumbs_t* thumbs_task = (task_check_thumbs_t*) malloc( sizeof( *thumbs_task ) );
    thumbs_task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_BUILD_DB, task_check_thumbs, cancel_task, thumbs_task );

    return true;
    }


static uint32_t musicdb_internal_add_genre( music_db_t* db, char const* genre_label )
    {
    uint32_t genre_id = MUSICDB_INVALID_ID;
    for( int j = 0; j < db->genres->header.count; ++j )
        {
        musicdb_genre_t* genre = &db->genres->items[ j ];
        if( genre->id == MUSICDB_INVALID_ID ) continue;
        if( strcmp( genre->label, genre_label ) == 0 )
            {
            genre_id = genre->id;
            break;
            }
        }

    if( genre_id == MUSICDB_INVALID_ID )
        {
        if( !musicdb_internal_ensure_genres_capacity( db ) ) return false;
           
        genre_id = (uint32_t) db->genres->header.count++;
        musicdb_genre_t* genre = &db->genres->items[ genre_id ];
        genre->id = genre_id ;
        MUSICDB_INTERNAL_STRNCPY( genre->label, genre_label );
        MUSICDB_INTERNAL_STRNCPY( genre->remapped_label, remap_genre( genre_label ) );

        char genre_pic[ 260 ];
        strcpy( genre_pic, "genres/" );
        char const* r1 = genre->label;
        char* w1 = genre_pic + strlen( genre_pic );
        while( *r1 )
            {
            char c = (char)tolower( *r1++ );
            if( c >= 'a' && c <= 'z' )  *w1++ = c;
            }
        *w1 = '\0';
        strcat( genre_pic, ".png" );

        if( file_exists( genre_pic ) )
            {
            bool added = false;
            genre->thumbnail_id = musicdb_internal_add_thumbnail( db, genre_pic , -1, &added );
            if( added )
                {
                task_generate_thumbnail_t* thumb_task = (task_generate_thumbnail_t*) malloc( sizeof( *thumb_task ) );
                thumb_task->db = db;
                thumb_task->id = genre->thumbnail_id;
                background_tasks_add( db->background_tasks, TASK_PRIORITY_GENERATE_THUMBNAIL_GENRE, task_generate_thumbnail, cancel_task, thumb_task );
                }
            }
        else
            {
            genre->thumbnail_id = MUSICDB_INVALID_ID;
            }

        char remap_genre_pic[ 260 ];
        strcpy( remap_genre_pic, "genres/" );
        char const* r2 = genre->remapped_label;
        char* w2 = remap_genre_pic + strlen( remap_genre_pic );
        while( *r2 )
            {
            char c = (char)tolower( *r2++ );
            if( c >= 'a' && c <= 'z' )  *w2++ = c;
            }
        *w2 = '\0';
        strcat( remap_genre_pic, ".png" );

        if( file_exists( remap_genre_pic ) )
            {
            bool added = false;
            genre->remapped_thumb_id = musicdb_internal_add_thumbnail( db, remap_genre_pic , -1, &added );
            if( added )
                {
                task_generate_thumbnail_t* thumb_task = (task_generate_thumbnail_t*) malloc( sizeof( *thumb_task ) );
                thumb_task->db = db;
                thumb_task->id = genre->remapped_thumb_id;
                background_tasks_add( db->background_tasks, TASK_PRIORITY_GENERATE_THUMBNAIL_GENRE, task_generate_thumbnail, cancel_task, thumb_task );
                }
            }
        else
            {
            genre->remapped_thumb_id = MUSICDB_INVALID_ID;
            }

        genre->album_count =0;
        }

    return genre_id;
    }


static uint32_t musicdb_internal_add_artist( music_db_t* db, char const* artist_name, char const* sort_name )
    {
    uint32_t artist_id = MUSICDB_INVALID_ID;
    for( int j = 0; j < db->artists->header.count; ++j )
        {
        musicdb_artist_t* artist = &db->artists->items[ j ];
        if( artist->id == MUSICDB_INVALID_ID ) continue;
        if( stricmp( artist->name, artist_name ) == 0 || stricmp( artist->name, sort_name ) == 0 )
            {
            artist_id = artist->id;
            break;
            }
        }

    if( artist_id == MUSICDB_INVALID_ID )
        {
        if( !musicdb_internal_ensure_artists_capacity( db ) ) return false;
           
        artist_id = (uint32_t) db->artists->header.count++;
        musicdb_artist_t* artist = &db->artists->items[ artist_id ];
        artist->id = artist_id;
        if( *sort_name )
            {
            MUSICDB_INTERNAL_STRNCPY( artist->name, sort_name );
            }
        else
            {
            MUSICDB_INTERNAL_STRNCPY( artist->name, artist_name );
            }
        artist->thumb_id = MUSICDB_INVALID_ID;                        
        artist->album_count =0;
        }

    return artist_id;
    }


static uint32_t musicdb_internal_add_album( music_db_t* db, char const* album_title, char const* album_artist, char const* sort_album_artist, char const* path, bool* added )
    {
    *added = false;
    uint32_t album_id = MUSICDB_INVALID_ID;
    for( int j = 0; j < db->albums->header.count; ++j )
        {
        musicdb_album_t* album = &db->albums->items[ j ];
        if( album->id == MUSICDB_INVALID_ID ) continue;
        if( strcmp( album->title, album_title ) == 0 && 
            ( *album->album_artist == 0 || *album_artist == 0 || strcmp( album->album_artist, album_artist ) == 0 ) &&
            ( *album->sort_artist == 0 || *sort_album_artist == 0 || strcmp( album->sort_artist, sort_album_artist ) == 0 ) && 
            ( strcmp( album->path, path ) == 0 ) )
            {
            album_id = album->id;
            break;
            }
        }

    if( album_id == MUSICDB_INVALID_ID )
        {
        if( !musicdb_internal_ensure_albums_capacity( db ) ) return false;
           
        album_id = (uint32_t) db->albums->header.count++;
        musicdb_album_t* album = &db->albums->items[ album_id ];
        album->id = album_id ;
        album->artist_id = MUSICDB_INVALID_ID;
        *album->artist = '\0';
        if( *album_artist ) MUSICDB_INTERNAL_STRNCPY( album->album_artist, album_artist );
        MUSICDB_INTERNAL_STRNCPY( album->sort_artist, sort_album_artist );
        MUSICDB_INTERNAL_STRNCPY( album->title, album_title );
        MUSICDB_INTERNAL_STRNCPY( album->path, path );
        album->thumb_id = MUSICDB_INVALID_ID;                        
        album->year = 0;
        album->disc_count = 0;
        album->track_count = 0;
        album->genres_count = 0;
        *added = true; 
        }

    return album_id;
    }


static void musicdb_internal_remove_artist( music_db_t* db, uint32_t artist_id )
    {
    if( (int)artist_id >= 0 && (int)artist_id < db->artists->header.count )
        {
        musicdb_artist_t* artist = &db->artists->items[ artist_id ];
        if( artist->id != MUSICDB_INVALID_ID )
            {
            memset( artist, 0, sizeof( *artist) );
            artist->id = MUSICDB_INVALID_ID;
            artist->thumb_id = MUSICDB_INVALID_ID;
            for( int i = 0; i < db->albums->header.count; ++i )
                {
                musicdb_album_t* album = &db->albums->items[ i ];
                if( album->id != MUSICDB_INVALID_ID && album->artist_id == artist_id )
                    {
                    for( int k = 0; k < album->genres_count; ++k )
                        {
                        uint32_t genre_id = album->genres_id[ k ];
                        if( (int) genre_id >= 0 && (int) genre_id < db->genres->header.count )
                            db->genres->items->album_count--;
                        }
                    uint32_t album_id = album->id;
                    memset( album, 0, sizeof( *album) );
                    album->id = MUSICDB_INVALID_ID;
                    album->artist_id = MUSICDB_INVALID_ID;
                    album->thumb_id = MUSICDB_INVALID_ID;
                    for( int j = 0; j < db->songs->header.count; ++j )
                        {
                        musicdb_song_t* song = &db->songs->items[ j ];
                        if( song->id != MUSICDB_INVALID_ID && song->album_id == album_id )
                            {
                            memset( song, 0, sizeof( *song ) );
                            song->id = MUSICDB_INVALID_ID;
                            song->album_id = MUSICDB_INVALID_ID;
                            song->mixtape_id = MUSICDB_INVALID_ID;
                            }
                        }
                    }
                }
            }
        }
    }


static void musicdb_internal_remove_album( music_db_t* db, uint32_t album_id )
    {
    if( (int)album_id >= 0 && (int)album_id < db->albums->header.count )
        {
        musicdb_album_t* album = &db->albums->items[ album_id ];
        if( album->id != MUSICDB_INVALID_ID )
            {
            for( int i = 0; i < album->genres_count; ++i )
                {
                uint32_t genre_id = album->genres_id[ i ];
                if( (int) genre_id >= 0 && (int) genre_id < db->genres->header.count )
                    db->genres->items[ genre_id ].album_count--;
                }
            uint32_t artist_id = album->artist_id;
            memset( album, 0, sizeof( *album) );
            album->id = MUSICDB_INVALID_ID;
            album->artist_id = MUSICDB_INVALID_ID;
            album->thumb_id = MUSICDB_INVALID_ID;
            if( (int)artist_id >= 0 && (int)artist_id < db->artists->header.count )
                {
                musicdb_artist_t* artist = &db->artists->items[ artist_id ];
                if( artist->id != MUSICDB_INVALID_ID )
                    {   
                    --artist->album_count;
                    if( artist->album_count <= 0 )
                        {
                        musicdb_internal_remove_artist( db, artist_id );
                        }
                    else 
                        {
                        if( (int)artist->thumb_id >= 0 && (int)artist->thumb_id < db->thumbnails->header.count )
                            db->thumbnails->items[ artist->thumb_id ].id = MUSICDB_INVALID_ID;
                        artist->thumb_id = MUSICDB_INVALID_ID;
                        task_create_artist_thumbnail_t* thumb_task = (task_create_artist_thumbnail_t*) malloc( sizeof( *thumb_task ) );
                        thumb_task->db = db;
                        thumb_task->id = artist->id;
                        thumb_task->album_count = artist->album_count;
                        background_tasks_add( db->background_tasks, TASK_PRIORITY_CREATE_ARTIST_THUMBNAIL, task_create_artist_thumbnail, cancel_task, thumb_task );
                        }
                    }
                }            
            }
        }
    }


static void musicdb_internal_remove_song( music_db_t* db, uint32_t song_id )
    {
    if( (int)song_id >= 0 && (int)song_id < db->songs->header.count )
        {
        musicdb_song_t* song = &db->songs->items[ song_id ];
        if( song->id != MUSICDB_INVALID_ID )
            {
            uint32_t album_id = song->album_id;
            memset( song, 0, sizeof( *song ) );
            song->id = MUSICDB_INVALID_ID;
            song->album_id = MUSICDB_INVALID_ID;
            song->mixtape_id = MUSICDB_INVALID_ID;
            if( (int)album_id >= 0 && (int)album_id < db->albums->header.count )
                {
                musicdb_album_t* album = &db->albums->items[ album_id ];
                if( album->id != MUSICDB_INVALID_ID )
                    {   
                    --album->track_count;
                    if( album->track_count <= 0 )
                        {
                        musicdb_internal_remove_album( db, album_id );
                        }
                    else
                        {
                        uint32_t artist_id = album->artist_id;
                        if( (int)artist_id >= 0 && (int)artist_id < db->artists->header.count )
                            {
                            musicdb_artist_t* artist = &db->artists->items[ artist_id ];
                            if( (int)artist->thumb_id >= 0 && (int)artist->thumb_id < db->thumbnails->header.count )
                                db->thumbnails->items[ artist->thumb_id ].id = MUSICDB_INVALID_ID;
                            artist->thumb_id = MUSICDB_INVALID_ID;
                            task_create_artist_thumbnail_t* thumb_task = (task_create_artist_thumbnail_t*) malloc( sizeof( *thumb_task ) );
                            thumb_task->db = db;
                            thumb_task->id = artist->id;
                            thumb_task->album_count = artist->album_count;
                            background_tasks_add( db->background_tasks, TASK_PRIORITY_CREATE_ARTIST_THUMBNAIL, task_create_artist_thumbnail, cancel_task, thumb_task );
                            }

                        if( (int)album->thumb_id >= 0 && (int)album->thumb_id < db->thumbnails->header.count )
                            db->thumbnails->items[ album->thumb_id ].id = MUSICDB_INVALID_ID;
                        album->thumb_id = MUSICDB_INVALID_ID;
                        task_create_album_thumbnail_t* thumb_task = (task_create_album_thumbnail_t*) malloc( sizeof( *thumb_task ) );
                        thumb_task->db = db;
                        thumb_task->id = album->id;
                        background_tasks_add( db->background_tasks, TASK_PRIORITY_CREATE_ALBUM_THUMBNAIL, task_create_album_thumbnail, cancel_task, thumb_task );
                        }
                    }
                }
            }
        }
    }


musicdb_genre_t* musicdb_genres_get( music_db_t* db, int* genre_count, int* total_album_count )
    {
    thread_mutex_lock( &db->mutex );

    *genre_count = db->genres->header.count;
    musicdb_genre_t* genres = (musicdb_genre_t*) malloc( sizeof( musicdb_genre_t ) * db->genres->header.count );
    int count = 0;
    for( int i = 0; i < db->genres->header.count; ++i )
        {
        musicdb_genre_t* genre = &db->genres->items[ i ];
        if( genre->id != MUSICDB_INVALID_ID && genre->album_count > 0 )
            genres[ count++ ] = *genre;
        }
    *genre_count = count;

    count = 0;
    for( int i = 0; i < db->albums->header.count; ++i )
        {
        musicdb_album_t* album = &db->albums->items[ i ];
        if( album->id != MUSICDB_INVALID_ID ) ++count;
        }
    *total_album_count = count;

    thread_mutex_unlock( &db->mutex );

    return genres;
    }


void musicdb_genres_release( music_db_t* db, musicdb_genre_t* genres )
    {
    (void) db;
    free( genres );
    }


static int sort_artists( void const* ptr_a, void const* ptr_b )
    { 
    musicdb_artist_t const* a = (musicdb_artist_t const*) ptr_a;
    musicdb_artist_t const* b = (musicdb_artist_t const*) ptr_b;
    char const* a_str = a->name;
    char const* b_str = b->name;
    if( stricmp( a_str, "Various Artists" ) == 0 && stricmp( b_str, "Various Artists" ) != 0 ) return 1;
    if( stricmp( a_str, "Various Artists" ) != 0 && stricmp( b_str, "Various Artists" ) == 0 ) return -1;
    if( strnicmp( a_str, "The ", strlen( "The " ) ) == 0 ) a_str += strlen( "The " );
    if( strnicmp( b_str, "The ", strlen( "The " ) ) == 0 ) b_str += strlen( "The " );
    return stricmp( a_str, b_str ); 
    } 


static int sort_songs( void const* ptr_a, void const* ptr_b )
    { 
    musicdb_song_t const* a = (musicdb_song_t const*) ptr_a;
    musicdb_song_t const* b = (musicdb_song_t const*) ptr_b;
    if( a->disc_number < b->disc_number ) return -1;
    if( a->disc_number > b->disc_number ) return 1;
    if( a->track_number < b->track_number ) return -1;
    if( a->track_number > b->track_number ) return 1;
    return 0; 
    } 


static int sort_albums( void const* ptr_a, void const* ptr_b )
    { 
    musicdb_album_t const* a = (musicdb_album_t const*) ptr_a;
    musicdb_album_t const* b = (musicdb_album_t const*) ptr_b;
    bool a_various = a->various;
    bool b_various = b->various;
    if( stricmp( a->artist, "Various Artists") == 0 ) a_various = true;
    if( stricmp( b->artist, "Various Artists") == 0 ) b_various = true;
    if( a_various && !b_various ) return 1;
    if( !a_various && b_various ) return -1;
    char const* a_str = *a->sort_artist ? a->sort_artist : a->artist;
    char const* b_str = *b->sort_artist ? b->sort_artist : b->artist;
    if( strnicmp( a_str, "The ", strlen( "The " ) ) == 0 ) a_str += strlen( "The " );
    if( strnicmp( b_str, "The ", strlen( "The " ) ) == 0 ) b_str += strlen( "The " );
    if( stricmp( a_str, b_str ) < 0 ) return -1;
    if( stricmp( a_str, b_str ) > 0 ) return 1;
    if( !a_various )
        {
        if( a->compilation && !b->compilation ) return 1;
        if( !a->compilation && b->compilation ) return -1;
        if( !a->compilation && !b->compilation ) 
            {
            if( a->year > 0 && b->year <= 0 ) return -1;
            if( b->year > 0 && a->year <= 0 ) return 1;
            if( a->year > 0 && a->year < b->year ) return -1;
            if( b->year > 0 && a->year > b->year ) return 1;
            }
        }
    a_str = a->title;
    b_str = b->title;
    if( strnicmp( a_str, "The ", strlen( "The " ) ) == 0 ) a_str += strlen( "The " );
    if( strnicmp( b_str, "The ", strlen( "The " ) ) == 0 ) b_str += strlen( "The " );
    return stricmp( a_str, b_str ); 
    } 


static int sort_artist_albums( void const* ptr_a, void const* ptr_b )
    { 
    musicdb_album_t const* a = (musicdb_album_t const*) ptr_a;
    musicdb_album_t const* b = (musicdb_album_t const*) ptr_b;
    bool a_various = a->various;
    bool b_various = b->various;
    if( stricmp( a->artist, "Various Artists") == 0 ) a_various = true;
    if( stricmp( b->artist, "Various Artists") == 0 ) b_various = true;
    if( a_various && !b_various ) return 1;
    if( !a_various && b_various ) return -1;
    if( !a_various )
        {
        if( a->compilation && !b->compilation )  return 1;
        if( !a->compilation && b->compilation )  return -1;
        if( !a->compilation && !b->compilation ) 
            {
            if( a->year > 0 && b->year <= 0 ) return -1;
            if( b->year > 0 && a->year <= 0 ) return 1;
            if( a->year > 0 && a->year < b->year ) return -1;
            if( b->year > 0 && a->year > b->year ) return 1;
            }
        }
    char const* a_str = a->title;
    char const* b_str = b->title;
    if( strnicmp( a_str, "The ", strlen( "The " ) ) == 0 ) a_str += strlen( "The " );
    if( strnicmp( b_str, "The ", strlen( "The " ) ) == 0 ) b_str += strlen( "The " );
    return stricmp( a_str, b_str ); 
    } 


musicdb_artist_t* musicdb_artists_get( music_db_t* db, uint32_t genre_id, bool remap_genre, int* artist_count, int* total_album_count )
    {
    thread_mutex_lock( &db->mutex );
   
    musicdb_artist_t* artists = (musicdb_artist_t*) malloc( sizeof( musicdb_artist_t ) * db->artists->header.count );

    int album_count = 0;
    if( genre_id == MUSICDB_INVALID_ID  )
        {
        int count = 0;
        for( int i = 0; i < db->artists->header.count; ++i )
            {
            musicdb_artist_t* artist = &db->artists->items[ i ];
            if( artist->id != MUSICDB_INVALID_ID && artist->album_count > 0 )
                {
                artists[ count++ ] = *artist;
                album_count += artist->album_count;
                }
            }
        *artist_count = count;
        }
    else if( remap_genre )
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            for( int j = 0; j < album->genres_count; ++j )
                {
                if( album->id != MUSICDB_INVALID_ID && strcmp( db->genres->items[ album->genres_id[ j ] ].remapped_label, db->genres->items[ genre_id ].remapped_label ) == 0 )
                    {
                    bool found = false;
                    for( int k = 0; k < count; ++k )
                        {
                        if( artists[ k ].id == album->artist_id )
                            {
                            found = true;
                            break;
                            }
                        }
                    musicdb_artist_t* artist = &db->artists->items[ album->artist_id ];
                    if( !found ) 
                        {
                        if( artist->id != MUSICDB_INVALID_ID && artist->album_count > 0 )
                            {
                            artists[ count++ ] = *artist;
                            if( stricmp( artist->name, "Various Artists" ) != 0 )
                                album_count += artist->album_count;
                            else
                                album_count++;
                            }
                        }
                    else if( stricmp( artist->name, "Various Artists" ) == 0 )
                        {
                        album_count++;
                        }
                    break;
                    }
                }
            }
        *artist_count = count;
        }
    else
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            for( int j = 0; j < album->genres_count; ++j )
                {
                if( album->id != MUSICDB_INVALID_ID && album->genres_id[ j ] == genre_id )
                    {
                    bool found = false;
                    for( int k = 0; k < count; ++k )
                        {
                        if( artists[ k ].id == album->artist_id )
                            {
                            found = true;
                            break;
                            }
                        }
                    musicdb_artist_t* artist = &db->artists->items[ album->artist_id ];
                    if( !found ) 
                        {
                        if( artist->id != MUSICDB_INVALID_ID && artist->album_count > 0 )
                            {
                            artists[ count++ ] = *artist;
                            if( stricmp( artist->name, "Various Artists" ) != 0 )
                                album_count += artist->album_count;
                            else
                                album_count++;
                            }
                        }
                    else if( stricmp( artist->name, "Various Artists" ) == 0 )
                        {
                        album_count++;
                        }
                    break;
                    }
                }
            }
        *artist_count = count;
        }

    *total_album_count = album_count;

    thread_mutex_unlock( &db->mutex );
    
    qsort( (void*)artists, *artist_count, sizeof( *artists ), sort_artists );
    return artists;
    }


void musicdb_artists_release( music_db_t* db, musicdb_artist_t* artists )
    {
    (void) db;
    free( artists );
    }



musicdb_song_t* musicdb_songs_get( music_db_t* db, uint32_t album_id, int* songs_count, musicdb_album_t* album_info, musicdb_thumbnail_t* album_thumb )
    {
    thread_mutex_lock( &db->mutex );

    if( (int)album_id < 0 || (int)album_id > db->albums->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    musicdb_album_t* album = &db->albums->items[ album_id ];
    if( album->id == MUSICDB_INVALID_ID )
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    if( album_info ) *album_info = *album;
    
    if( album_thumb )
        {
        album_thumb->id = MUSICDB_INVALID_ID;

        if( (int)album->thumb_id >= 0 && (int)album->thumb_id < db->thumbnails->header.count )
            *album_thumb = db->thumbnails->items[ album->thumb_id ];
        }
   
    musicdb_song_t* songs = (musicdb_song_t*) malloc( sizeof( musicdb_song_t ) * 256 );
    int count = 0;
    for( int i = 0; i < db->songs->header.count; ++i )
        {
        musicdb_song_t* song = &db->songs->items[ i ];
        if( song->id != MUSICDB_INVALID_ID && song->album_id == album_id )
            songs[ count++ ] = *song;
        if( count >= 256 ) break;
        }
    *songs_count = count;
   
    thread_mutex_unlock( &db->mutex );
    
    qsort( (void*)songs, count, sizeof( *songs ), sort_songs );
    return songs;
    }


void musicdb_songs_release( music_db_t* db, musicdb_song_t* songs )
    {
    (void) db;
    free( songs );
    }



bool musicdb_artist_info( music_db_t* db, uint32_t artist_id, musicdb_artist_t* artist_info )
    {
    thread_mutex_lock( &db->mutex );

    if( (int)artist_id < 0 || (int)artist_id > db->artists->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return false;
        }

    musicdb_artist_t* artist = &db->artists->items[ artist_id ];
    if( artist->id == MUSICDB_INVALID_ID )
        {
        thread_mutex_unlock( &db->mutex );
        return false;
        }

    *artist_info = *artist;
   
    thread_mutex_unlock( &db->mutex );
    return true;
    }


bool musicdb_genre_info( music_db_t* db, uint32_t genre_id, musicdb_genre_t* genre_info )
    {
    thread_mutex_lock( &db->mutex );

    if( (int)genre_id < 0 || (int)genre_id > db->genres->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return false;
        }

    musicdb_genre_t* genre = &db->genres->items[ genre_id ];
    if( genre->id == MUSICDB_INVALID_ID )
        {
        thread_mutex_unlock( &db->mutex );
        return false;
        }

    *genre_info = *genre;
   
    thread_mutex_unlock( &db->mutex );
    return true;
    }


musicdb_album_t* musicdb_albums_get( music_db_t* db, uint32_t genre_id, uint32_t artist_id, bool remap_genre, int* album_count )
    {
    thread_mutex_lock( &db->mutex );
    musicdb_album_t* albums = (musicdb_album_t*) malloc( sizeof( musicdb_album_t ) * db->albums->header.count );
    if( genre_id == MUSICDB_INVALID_ID && artist_id == MUSICDB_INVALID_ID )
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            if( album->id != MUSICDB_INVALID_ID )
                albums[ count++ ] = *album;
            }
        *album_count = count;
        }
    else if( genre_id == MUSICDB_INVALID_ID && artist_id != MUSICDB_INVALID_ID )
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            if( album->id != MUSICDB_INVALID_ID && album->artist_id == artist_id )
                albums[ count++ ] = *album;
            }
        *album_count = count;
        }
    else if( remap_genre && artist_id != MUSICDB_INVALID_ID )
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            if( album->id != MUSICDB_INVALID_ID && album->artist_id == artist_id )
                {
                if( stricmp( album->artist, "Various Artists" ) == 0 ) 
                    {
                    for( int j = 0; j < album->genres_count; ++j )
                        {
                        if( strcmp( db->genres->items[ album->genres_id[ j ] ].remapped_label, db->genres->items[ genre_id ].remapped_label ) == 0 )
                            {
                            albums[ count++ ] = *album;
                            break;
                            }
                        }
                    }
                else
                    {
                    albums[ count++ ] = *album;
                    }
                }
            }
        *album_count = count;
        }
    else if( remap_genre && artist_id == MUSICDB_INVALID_ID )
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            for( int j = 0; j < album->genres_count; ++j )
                {
                if( album->id != MUSICDB_INVALID_ID && strcmp( db->genres->items[ album->genres_id[ j ] ].remapped_label, db->genres->items[ genre_id ].remapped_label ) == 0 )
                    {
                    albums[ count++ ] = *album;
                    break;
                    }
                }
            }
        *album_count = count;
        }
    else if( !remap_genre && artist_id != MUSICDB_INVALID_ID )
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            if( album->id != MUSICDB_INVALID_ID && album->artist_id == artist_id )
                {
                if( stricmp( album->artist, "Various Artists" ) == 0 ) 
                    {
                    for( int j = 0; j < album->genres_count; ++j )
                        {
                        if( album->genres_id[ j ] == genre_id )
                            {
                            albums[ count++ ] = *album;
                            break;
                            }
                        }
                    }
                else
                    {
                    albums[ count++ ] = *album;
                    }
                }
            }
        *album_count = count;
        }
    else
        {
        int count = 0;
        for( int i = 0; i < db->albums->header.count; ++i )
            {
            musicdb_album_t* album = &db->albums->items[ i ];
            for( int j = 0; j < album->genres_count; ++j )
                {
                if( album->id != MUSICDB_INVALID_ID && album->genres_id[ j ] == genre_id )
                    {
                    albums[ count++ ] = *album;
                    break;
                    }
                }
            }
        *album_count = count;
        }
    thread_mutex_unlock( &db->mutex );
    
    if( artist_id == MUSICDB_INVALID_ID )
        qsort( (void*)albums, *album_count, sizeof( *albums ), sort_albums );
    else
        qsort( (void*)albums, *album_count, sizeof( *albums ), sort_artist_albums );
    return albums;
    }


void musicdb_albums_release( music_db_t* db, musicdb_album_t* albums )
    {
    (void) db;
    free( albums );
    }


typedef struct musicdb_shuffle_song_t
    {
    uint32_t song_id;
    char title[ 128 ];
    char album[ 128 ];
    char artist[ 128 ];
    int length_in_seconds;
    int year;
    bitmap_t* thumbnail;
    int thumb_width;
    int thumb_height;
    int thumb_mip_width;
    int thumb_mip_height;
    } musicdb_shuffle_song_t;


musicdb_song_t* musicdb_shuffle_get( music_db_t* db, int* songs_count, int start, int max_count )
    {
    thread_mutex_lock( &db->mutex );

    int end = max_count > 0 ? start + max_count : db->shuffle->header.count;
    if( end > db->shuffle->header.count ) end = db->shuffle->header.count;
    musicdb_song_t* shuffles = (musicdb_song_t*) malloc( sizeof( musicdb_song_t ) * (end - start) );
    memset( shuffles, 0, sizeof( musicdb_song_t ) * (end - start) );
    int count = 0;
    for( int i = 0; i < end - start; ++i )
        {
        int song_index = db->shuffle->items[ i + start ];
        if( song_index >= 0 && song_index < db->songs->header.count ) 
            {
            musicdb_song_t* song = &db->songs->items[ song_index ];
            shuffles[ count++ ] = *song;
            }
        }
    *songs_count = count;
   
    thread_mutex_unlock( &db->mutex );   
    return shuffles;
    }


void musicdb_shuffle_release( music_db_t* db, musicdb_song_t* shuffle_songs )
    {
    (void) db;
    free( shuffle_songs );
    }


void musicdb_shuffle_add_album( music_db_t* db, uint32_t album_id ) 
    {
    thread_mutex_lock( &db->mutex );

    if( (int)album_id < 0 || (int)album_id >= db->albums->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return;
        }

    musicdb_album_t* album = &db->albums->items[ album_id ];
    if( album->id == MUSICDB_INVALID_ID )
        {
        thread_mutex_unlock( &db->mutex );
        return;
        }
 
    for( int i = 0; i < db->songs->header.count; ++i )
        {
        musicdb_song_t* song = &db->songs->items[ i ];
        if( song->id != MUSICDB_INVALID_ID && song->album_id == album_id ) 
            {
            bool found = false;
            for( int j = 0; j < db->shuffle->header.count; ++j ) 
                {
                if( db->shuffle->items[ j ] == i ) 
                    {
                    found = true;
                    break;
                    }
                }
            if( !found ) 
                {
                musicdb_internal_ensure_shuffle_capacity( db );
                db->shuffle->items[ db->shuffle->header.count++ ] = i;
                }
            }
        }
   
    thread_mutex_unlock( &db->mutex ); 
    
    task_write_shuffle_list_t* write_task = (task_write_shuffle_list_t*) malloc( sizeof( *write_task ) );
    write_task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_WRITE_SHUFFLE_LIST, task_write_shuffle_list, cancel_task, write_task );
    }


void musicdb_shuffle_add_genre( music_db_t* db, uint32_t genre_id ) 
    {
    thread_mutex_lock( &db->mutex );

    if( (int)genre_id < 0 || (int)genre_id >= db->genres->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return;
        }

    for( int j = 0; j < db->albums->header.count; ++j )
        {
        musicdb_album_t* album = &db->albums->items[ j ];
        if( album->id == MUSICDB_INVALID_ID )
            {
            continue;
            }
                
        bool genre = false;
        for( int k = 0; k < album->genres_count; ++k ) 
            {
            if( album->genres_id[ k ] == genre_id ) 
                {
                genre = true;
                break;
                }
            }
 
        if( !genre ) continue;
            
        for( int i = 0; i < db->songs->header.count; ++i )
            {
            musicdb_song_t* song = &db->songs->items[ i ];
            if( song->id != MUSICDB_INVALID_ID && song->album_id == album->id && ( song->length_in_seconds == 0 || song->length_in_seconds > 90 ) && song->track_gain != 1.0f ) 
                {
                bool found = false;
                for( int k = 0; k < db->shuffle->header.count; ++k ) 
                    {
                    if( db->shuffle->items[ k ] == i ) 
                        {
                        found = true;
                        break;
                        }
                    }
                if( !found ) 
                    {
                    musicdb_internal_ensure_shuffle_capacity( db );
                    db->shuffle->items[ db->shuffle->header.count++ ] = i;
                    }
                }
            }
        }   
    thread_mutex_unlock( &db->mutex ); 
    
    task_write_shuffle_list_t* write_task = (task_write_shuffle_list_t*) malloc( sizeof( *write_task ) );
    write_task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_WRITE_SHUFFLE_LIST, task_write_shuffle_list, cancel_task, write_task );
    }

void musicdb_shuffle_remove( music_db_t* db, int index ) 
    {
    thread_mutex_lock( &db->mutex );

    if( index < 0 || index >= db->shuffle->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return;
        }
    
    --db->shuffle->header.count;
    if( index < db->shuffle->header.count ) {
        memmove( &db->shuffle->items[ index ], &db->shuffle->items[ index + 1 ], ( db->shuffle->header.count - index ) * sizeof( *db->shuffle->items ) );
    }
    thread_mutex_unlock( &db->mutex ); 

    task_write_shuffle_list_t* write_task = (task_write_shuffle_list_t*) malloc( sizeof( *write_task ) );
    write_task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_WRITE_SHUFFLE_LIST, task_write_shuffle_list, cancel_task, write_task );
}


void musicdb_shuffle_add_all( music_db_t* db ) 
    {
    thread_mutex_lock( &db->mutex );
    db->shuffle->header.count = 0;
    for( int i = 0; i < db->songs->header.count; ++i )
        {
        musicdb_song_t* song = &db->songs->items[ i ];
        if( song->id != MUSICDB_INVALID_ID && ( song->length_in_seconds == 0 || song->length_in_seconds > 90 ) && song->track_gain != 1.0f ) 
            {
            musicdb_internal_ensure_shuffle_capacity( db );
            db->shuffle->items[ db->shuffle->header.count++ ] = i;
            }
        }
   
    thread_mutex_unlock( &db->mutex ); 
    
    task_write_shuffle_list_t* write_task = (task_write_shuffle_list_t*) malloc( sizeof( *write_task ) );
    write_task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_WRITE_SHUFFLE_LIST, task_write_shuffle_list, cancel_task, write_task );
    }

void musicdb_shuffle_remove_all( music_db_t* db ) {
    thread_mutex_lock( &db->mutex );
    db->shuffle->header.count = 0;
    thread_mutex_unlock( &db->mutex ); 

    task_write_shuffle_list_t* write_task = (task_write_shuffle_list_t*) malloc( sizeof( *write_task ) );
    write_task->db = db;
    background_tasks_add( db->background_tasks, TASK_PRIORITY_WRITE_SHUFFLE_LIST, task_write_shuffle_list, cancel_task, write_task );
}

void musicdb_shuffle_reshuffle( music_db_t* db ) {
    thread_mutex_lock( &db->mutex );
    shuffle_ints( db->shuffle->items, db->shuffle->header.count );  
    thread_mutex_unlock( &db->mutex ); 
}


bitmap_t* musicdb_get_thumbnail( music_db_t* db, uint32_t id, int* w, int* h, int* mip_width, int* mip_height, render_t* render )
    {
    thread_mutex_lock( &db->mutex );
    if( (int)id < 0 || (int)id > db->thumbnails->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    musicdb_thumbnail_t* thumb = &db->thumbnails->items[ id ];
    if( !thumb->initialized )
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    #ifdef MUSICDB_THUMBNAIL_FILES

        mmap_t* mmap = mmap_open_read_only( thumb->thumbs_file, file_size( thumb->thumbs_file ) );
        if( !mmap || mmap_size( mmap ) < sizeof( uint32_t) * 2 )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }

       uint32_t* ptr = (uint32_t*) mmap_data( mmap );
        if( !ptr )
            {
            mmap_close( mmap );
            thread_mutex_unlock( &db->mutex );
            return 0;
            }

         if( mmap_size( mmap ) < sizeof( uint32_t ) * 2 ) 
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
    #else
        if( !thumb->pixels_offset )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
        uint32_t* ptr = db->pixels->pixel_data + thumb->pixels_offset;
    #endif

    *mip_width = (int)*ptr++;
    *mip_height = (int)*ptr++;

    #ifdef MUSICDB_THUMBNAIL_FILES
        if( mmap_size( mmap ) < sizeof( uint32_t ) * ( 2 + calculate_mip_size( *mip_width, *mip_height ) ) )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
    #endif

    *w = thumb->width;
    *h = thumb->height;
    bitmap_t* bitmap = bitmap_create( render, *w, *h,ptr, *mip_width, *mip_height );
    thread_mutex_unlock( &db->mutex );
    return bitmap;
    }


bitmap_t* musicdb_get_album_thumbnail( music_db_t* db, uint32_t album_id, int* w, int* h, int* mip_width, int* mip_height, render_t* render )
    {
    thread_mutex_lock( &db->mutex );

    if( (int)album_id < 0 || (int)album_id > db->albums->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }
    
    uint32_t id = db->albums->items[ album_id ].thumb_id;

    if( id == MUSICDB_INVALID_ID || (int)id < 0 || (int)id > db->thumbnails->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    musicdb_thumbnail_t* thumb = &db->thumbnails->items[ id ];
    if( !thumb->initialized )
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    #ifdef MUSICDB_THUMBNAIL_FILES
        mmap_t* mmap = mmap_open_read_only( thumb->thumbs_file, file_size( thumb->thumbs_file ) );
        if( !mmap || mmap_size( mmap ) < sizeof( uint32_t) * 2 )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }

        uint32_t* ptr = (uint32_t*) mmap_data( mmap );
        if( !ptr )
            {
            mmap_close( mmap );
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
    #else
        if( !thumb->pixels_offset )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
        uint32_t* ptr = db->pixels->pixel_data + thumb->pixels_offset;
    #endif

    *mip_width = (int)*ptr++;
    *mip_height = (int)*ptr++;

    #ifdef MUSICDB_THUMBNAIL_FILES
        if( mmap_size( mmap ) < sizeof( uint32_t ) * ( 2 + calculate_mip_size( *mip_width, *mip_height ) ) )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
    #endif

    *w = thumb->width;
    *h = thumb->height;
    bitmap_t* bitmap = bitmap_create( render, *w, *h,ptr, *mip_width, *mip_height );
    thread_mutex_unlock( &db->mutex );
    return bitmap;
    }


bitmap_t* musicdb_get_artist_thumbnail( music_db_t* db, uint32_t artist_id, int* w, int* h, int* mip_width, int* mip_height, render_t* render )
    {
    thread_mutex_lock( &db->mutex );

    if( (int)artist_id < 0 || (int)artist_id > db->artists->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }
    
    uint32_t id = db->artists->items[ artist_id ].thumb_id;

    if( id == MUSICDB_INVALID_ID || (int)id < 0 || (int)id > db->thumbnails->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    musicdb_thumbnail_t* thumb = &db->thumbnails->items[ id ];
    if( !thumb->initialized )
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    #ifdef MUSICDB_THUMBNAIL_FILES
        mmap_t* mmap = mmap_open_read_only( thumb->thumbs_file, file_size( thumb->thumbs_file ) );
        if( !mmap || mmap_size( mmap ) < sizeof( uint32_t) * 2 )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }

        uint32_t* ptr = (uint32_t*) mmap_data( mmap );
        if( !ptr )
            {
            mmap_close( mmap );
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
    #else
        if( !thumb->pixels_offset )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }

        uint32_t* ptr = db->pixels->pixel_data + thumb->pixels_offset;
    #endif

    *mip_width = (int)*ptr++;
    *mip_height = (int)*ptr++;

    #ifdef MUSICDB_THUMBNAIL_FILES
        if( mmap_size( mmap ) < sizeof( uint32_t ) * ( 2 + calculate_mip_size( *mip_width, *mip_height ) ) )
            {
            thread_mutex_unlock( &db->mutex );
            return 0;
            }
    #endif

    *w = thumb->width;
    *h = thumb->height;
    bitmap_t* bitmap = bitmap_create( render, *w, *h,ptr, *mip_width, *mip_height );
    thread_mutex_unlock( &db->mutex );
    return bitmap;
    }


bool musicdb_is_thumbnail_initialized( music_db_t* db, uint32_t thumb_id )
    {
    thread_mutex_lock( &db->mutex );

    if( (int)thumb_id < 0 || (int)thumb_id > db->thumbnails->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return 0;
        }

    bool result = db->thumbnails->items[ thumb_id ].initialized;

    thread_mutex_unlock( &db->mutex );
    return result;
    }


void musicdb_update_song_length( music_db_t* db, uint32_t song_id, int length_in_seconds )
    {
    thread_mutex_lock( &db->mutex );

    if( (int)song_id < 0 || (int)song_id > db->songs->header.count ) 
        {
        thread_mutex_unlock( &db->mutex );
        return;
        }

    if( db->songs->items[ song_id ].id != MUSICDB_INVALID_ID ) 
        {
        db->songs->items[ song_id ].length_in_seconds = length_in_seconds;
        }

    thread_mutex_unlock( &db->mutex );
    }

