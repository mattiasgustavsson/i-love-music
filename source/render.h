
typedef struct tex_t
    { 
    GLuint handle;
    int width;
    int height;
    bool mipmaps;
    } tex_t;

typedef struct render_t
    {
    app_t* app;
    float logical_width;
    float logical_height;
    float view_width;
    float view_height;
    float yoffset;

    GLuint shader;	
	GLuint vertexbuffer;
    GLuint white_tex;

    int textures_count;
    int textures_capacity;
    tex_t* textures;
    
    } render_t;


void render_yoffset( render_t* render, float yoffset )
    {
    render->yoffset = yoffset;
    }


bool render_init( render_t* render, app_t* app, int logical_width, int logical_height )
    {
    memset( render, 0, sizeof( *render ) );
    render->app = app;
    render->logical_width = (float) logical_width;
    render->logical_height = (float) logical_height;
	
    int viewport[ 4 ];
	glGetIntegerv( GL_VIEWPORT, viewport );
    render->view_width = (float)( viewport[ 2 ] - viewport[ 0 ] );
    render->view_height = (float)( viewport[ 3 ] - viewport[ 1 ] );

	#define STR( x ) #x

	char const* vs_source = 
	    STR(
		    #version 330\n\n

		    layout( location = 0 ) in vec4 pos;
		    out vec2 uv;

		    void main( void )
			    {
			    gl_Position = vec4( pos.xy, 0.0, 1.0 );
			    uv = pos.zw;
			    }
	    ) /* STR */;

	char const* fs_source = 
	    STR (
		    #version 330\n\n

		    in vec2 uv;
		    out vec4 color;

    		uniform vec4 col;
		    uniform sampler2D tex0;

		    void main( void )
	            {
	            color = texture2D( tex0, uv ) * col;
	            }	
	    ) /* STR */;

    #undef STR

	char error_message[ 1024 ]; 

	GLuint vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, (char const**) &vs_source, NULL );
	glCompileShader( vs );
	GLint vs_compiled;
	glGetShaderiv( vs, GL_COMPILE_STATUS, &vs_compiled );
	if( !vs_compiled )
		{
		char const* prefix = "Vertex Shader Error: ";
		strcpy( error_message, prefix );
		int len = 0, written = 0;
		glGetShaderiv( vs, GL_INFO_LOG_LENGTH, &len );
		glGetShaderInfoLog( vs, (GLsizei)( sizeof( error_message ) - strlen( prefix ) ), &written, 
			error_message + strlen( prefix ) );		
		app_fatal_error( app, error_message );
		return false;
		}
	
	GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, (char const**) &fs_source, NULL );
	glCompileShader( fs );
	GLint fs_compiled;
	glGetShaderiv( fs, GL_COMPILE_STATUS, &fs_compiled );
	if( !fs_compiled )
		{
		char const* prefix = "Fragment Shader Error: ";
		strcpy( error_message, prefix );
		int len = 0, written = 0;
		glGetShaderiv( vs, GL_INFO_LOG_LENGTH, &len );
		glGetShaderInfoLog( fs, (GLsizei)( sizeof( error_message ) - strlen( prefix ) ), &written, 
			error_message + strlen( prefix ) );		
		app_fatal_error( app, error_message );
		return false;
		}


	GLuint prg = glCreateProgram();
	glAttachShader( prg, fs );
	glAttachShader( prg, vs );
	glBindAttribLocation( prg, 0, "pos" );
	glLinkProgram( prg );

	GLint linked;
	glGetProgramiv( prg, GL_LINK_STATUS, &linked );
	if( !linked )
		{
		char const* prefix = "Shader Link Error: ";
		strcpy( error_message, prefix );
		int len = 0, written = 0;
		glGetShaderiv( vs, GL_INFO_LOG_LENGTH, &len );
		glGetShaderInfoLog( prg, (GLsizei)( sizeof( error_message ) - strlen( prefix ) ), &written, 
			error_message + strlen( prefix ) );		
		app_fatal_error( app, error_message );
		return false;
		}

    render->shader = prg;

	glGenBuffers( 1, &render->vertexbuffer );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( GLfloat ), 0 );

    glDisable( GL_DEPTH_TEST );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glGenTextures( 1, &render->white_tex );
    glActiveTexture( GL_TEXTURE0 ); 
    glBindTexture( GL_TEXTURE_2D, render->white_tex );
    uint32_t pixel = 0xffffffff;
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixel ); 

    render->textures_count = 0;
    render->textures_capacity = 256;
    render->textures = (tex_t*) malloc( sizeof( tex_t ) * render->textures_capacity );
    return true;
    }


void render_term( render_t* render )
    {
    for( int i = 0; i < render->textures_count; ++i )
        glDeleteTextures( 1, &render->textures[ i ].handle );
    free( render->textures );
	glDeleteShader( render->shader );
	glDeleteBuffers( 1, &render->vertexbuffer );
    }

void render_scissor( render_t* render, int x1, int y1, int x2, int y2 )
    {    
    y1 = (int) render->logical_height - y1;
    y2 = (int) render->logical_height - y2;
    int ix = (int)( ( x1 / render->logical_width ) * render->view_width );
    int iy = (int)( ( y2 / render->logical_height ) * render->view_height );
    int iw = (int)( ( (x2 - x1) / render->logical_width ) * render->view_width );
    int ih = (int)( ( (y1 - y2) / render->logical_height ) * render->view_height );

    glEnable( GL_SCISSOR_TEST );
    glScissor( ix, iy, iw, ih );
    }

void render_scissor_off( render_t* render )
    {    
    (void) render;
    glDisable( GL_SCISSOR_TEST );
    }


tex_t render_internal_allocate_texture( render_t* render, int width, int height, bool mipmaps )
    {
    int best_index = -1;
    for( int i = 0; i < render->textures_count; ++i )
        {
        tex_t* tex = &render->textures[ i ];
        if( tex->width == width && tex->height == height && tex->mipmaps == mipmaps )
            {
            best_index = i;
            break;
            }
        }
    
    if( best_index >= 0 )
        { 
        tex_t tex = render->textures[ best_index ];
        render->textures[ best_index ] = render->textures[ --render->textures_count ];
        return tex;
        }

    static uint32_t empty[ 4096 * 4096 ];
    GLuint handle;
    glGenTextures( 1, &handle );
    glActiveTexture( GL_TEXTURE0 ); 
    glBindTexture( GL_TEXTURE_2D, handle );
    int level = 0;
    int w = width;
    int h = height;
    while( w > 0 && h > 0 )
        {
        glTexImage2D( GL_TEXTURE_2D, level++, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, empty ); 
        w /= 2;
        h /= 2;
        if( !mipmaps ) break;
        }
    tex_t tex;
    tex.handle = handle;
    tex.width = width;
    tex.height = height;
    tex.mipmaps = mipmaps;
    return tex;
    }


void render_internal_free_texture( render_t* render, tex_t tex )
    {
    if( render->textures_count >= render->textures_capacity )
        {
        render->textures_capacity *= 2;
        render->textures = (tex_t*) realloc( render->textures, sizeof( tex_t ) * render->textures_capacity );
        }
   render->textures[ render->textures_count++ ] = tex;
   }


bool render_view_valid( render_t* render )
    {
    (void) render;
    int viewport[ 4 ];
	glGetIntegerv( GL_VIEWPORT, viewport );
    float view_width = (float)( viewport[ 2 ] - viewport[ 0 ] );
    float view_height = (float)( viewport[ 3 ] - viewport[ 1 ] );
    return view_width > 32.0f && view_height > 32.0f;
    }

bool render_update_window( render_t* render, int logical_width, int logical_height )
    {
    bool res = false;
    if( logical_width != render->logical_width || logical_height != render->logical_height ) res = true;

    render->logical_width = (float) logical_width;
    render->logical_height = (float) logical_height;

    int viewport[ 4 ];
	glGetIntegerv( GL_VIEWPORT, viewport );
    float view_width = (float)( viewport[ 2 ] - viewport[ 0 ] );
    float view_height = (float)( viewport[ 3 ] - viewport[ 1 ] );
    if( view_width != render->view_width || view_height != render->view_height ) res = true;
    render->view_width = view_width;
    render->view_height = view_height;

    if( render->view_height < 2.0f ) 
        {
        render->view_height = 2.0f;
        render->view_width = 2.0f * ( render->logical_width / render->logical_height );
        res = true;
        }

    return res;
    }


void render_new_frame( render_t* render )
    {
    (void) render;
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
    glClear( GL_COLOR_BUFFER_BIT );
    }


void render_draw_rect( render_t* render, int x, int y, int w, int h, uint32_t color )
    {
    y -= (int)render->yoffset;
    int ix = (int)( ( x / render->logical_width ) * render->view_width );
    int iy = (int)( ( y / render->logical_height ) * render->view_height );
    int iw = (int)( ( w / render->logical_width ) * render->view_width );
    int ih = (int)( ( h / render->logical_height ) * render->view_height );
	float x1 = ix / render->view_width;
    float y1 = iy / render->view_height;
    float x2 = x1 + iw / render->view_width;
    float y2 = y1 + ih / render->view_height;

	GLfloat vertices[] = 
		{ 
		2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		};
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, render->white_tex );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    }

void render_draw_rectf( render_t* render, float x, float y, float w, float h, uint32_t color )
    {
    y -= (int)render->yoffset;
	float x1 = x / render->logical_width;
    float y1 = y / render->logical_height;
    float x2 = x1 + w / render->logical_width;
    float y2 = y1 + h / render->logical_height;

	GLfloat vertices[] = 
		{ 
		2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		};
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, render->white_tex );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    }

    
typedef struct bitmap_t
    {
    tex_t tex;
    int width;
    int height;
    } bitmap_t;


bitmap_t* bitmap_create_raw( render_t* render, uint32_t const* pixels, int width, int height )
    {
    (void) render, pixels;
    bitmap_t* bmp = (bitmap_t* )malloc( sizeof( bitmap_t ) );
    memset( bmp, 0, sizeof( *bmp ) );

    bmp->width = width;
    bmp->height = height;

    GLuint handle;
    glGenTextures( 1, &handle );
    glActiveTexture( GL_TEXTURE0 ); 
    glBindTexture( GL_TEXTURE_2D, handle );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels ); 
    if( glGenerateMipmap ) glGenerateMipmap( GL_TEXTURE_2D );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR  );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glBindTexture( GL_TEXTURE_2D, 0 );   
    bmp->tex.handle = handle;
    bmp->tex.width = width;
    bmp->tex.height = height;
    bmp->tex.mipmaps = false;
    return bmp;
    }


bitmap_t* bitmap_create( render_t* render, int width, int height, uint32_t const* mipmaps, int mipmaps_width, int mipmaps_height )
    {
    (void) render;
    bitmap_t* bmp = (bitmap_t* )malloc( sizeof( bitmap_t ) );
    memset( bmp, 0, sizeof( *bmp ) );

    bmp->width = width;
    bmp->height = height;

    
    bmp->tex = render_internal_allocate_texture( render, mipmaps_width, mipmaps_height, true );
    glActiveTexture( GL_TEXTURE0 ); 
    glBindTexture( GL_TEXTURE_2D, bmp->tex.handle );
    
    int level = 0;
    while( mipmaps_width > 0 && mipmaps_height > 0 )
        {
        glTexSubImage2D( GL_TEXTURE_2D, level, 0, 0, mipmaps_width, mipmaps_height, GL_RGBA, GL_UNSIGNED_BYTE, mipmaps ); 
        ++level;
        mipmaps += mipmaps_width * mipmaps_height;
        mipmaps_width /= 2;
        mipmaps_height /= 2;
        }
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level - 1 );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR  );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glBindTexture( GL_TEXTURE_2D, 0 );   

    return bmp;
    }


void bitmap_destroy( render_t* render, bitmap_t* bmp )
    {
    if( !bmp ) return;
    (void) render;
    render_internal_free_texture( render, bmp->tex );
    free( bmp );
    }


void bitmap_draw( render_t* render, bitmap_t* bmp, int x, int y, int w, int h, uint32_t color )
    {
    y -= (int)render->yoffset;
    int xofs = 0;
    int yofs = 0;
    if( bmp->width > bmp->height )
        {
        float scale = bmp->height / (float) bmp->width;
        int height = (int)( h * scale );
        yofs = h - height;
        }
    else if( bmp->height > bmp->width )
        {
        float scale = bmp->width / (float) bmp->height;
        int width = (int)( w * scale );
        xofs = w - width;
        }

    x += xofs / 2;
    w -= xofs;
    y += yofs / 2;
    h -= yofs;

    int ix = (int)( ( x / render->logical_width ) * render->view_width );
    int iy = (int)( ( y / render->logical_height ) * render->view_height );
    int iw = (int)( ( w / render->logical_width ) * render->view_width );
    int ih = (int)( ( h / render->logical_height ) * render->view_height );
	float x1 = ix / render->view_width;
    float y1 = iy / render->view_height;
    float x2 = x1 + iw / render->view_width;
    float y2 = y1 + ih / render->view_height;

	GLfloat vertices[] = 
		{ 
		2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		};
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, bmp->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, 0 );   
    }


void rotate_points( float radians, float* vertices ) 
    {  
    float c = cosf( radians );
    float s = sinf( radians );

    for( int i = 0; i < 4; ++i ) 
        {
        float x = vertices[ i * 2 + 0 ];
        float y = vertices[ i * 2 + 1 ];
        vertices[ i * 2 + 0 ] = ( x * c - y * s );
        vertices[ i * 2 + 1 ] = ( x * s + y * c );
        }
    }


void bitmap_draw_rotated( render_t* render, bitmap_t* bmp, int x, int y, int w, int h, float radians, uint32_t color )
    {
    y -= (int)render->yoffset;
    int xofs = 0;
    int yofs = 0;
    if( bmp->width > bmp->height )
        {
        float scale = bmp->height / (float) bmp->width;
        int height = (int)( h * scale );
        yofs = h - height;
        }
    else if( bmp->height > bmp->width )
        {
        float scale = bmp->width / (float) bmp->height;
        int width = (int)( w * scale );
        xofs = w - width;
        }

    x += xofs / 2;
    w -= xofs;
    y += yofs / 2;
    h -= yofs;

    int ix = (int)( ( x / render->logical_width ) * render->view_width );
    int iy = (int)( ( y / render->logical_height ) * render->view_height );
    int iw = (int)( ( w / render->logical_width ) * render->view_width );
    int ih = (int)( ( h / render->logical_height ) * render->view_height );

    float points[] = 
    {
        -iw / 2.0f, -ih / 2.0f,
         iw / 2.0f, -ih / 2.0f,
         iw / 2.0f,  ih / 2.0f,
        -iw / 2.0f,  ih / 2.0f,
    };
    rotate_points( radians, points );

	GLfloat vertices[] = 
		{ 
		( ix + points[ 0 ] + iw / 2.0f ) / render->view_width, ( iy + points[ 1 ] + ih / 2.0f) / render->view_height, 0.0f, 0.0f,
		( ix + points[ 2 ] + iw / 2.0f ) / render->view_width, ( iy + points[ 3 ] + ih / 2.0f) / render->view_height, 1.0f, 0.0f,
		( ix + points[ 4 ] + iw / 2.0f ) / render->view_width, ( iy + points[ 5 ] + ih / 2.0f) / render->view_height, 1.0f, 1.0f,
		( ix + points[ 6 ] + iw / 2.0f ) / render->view_width, ( iy + points[ 7 ] + ih / 2.0f) / render->view_height, 0.0f, 1.0f,
		};

    for( int i = 0; i < 4; ++i ) 
		{ 
        vertices[ i * 4 + 0 ] = vertices[ i * 4 + 0 ] * 2.0f - 1.0f;
        vertices[ i * 4 + 1 ] = vertices[ i * 4 + 1 ] * -2.0f + 1.0f;
		}
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, bmp->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, 0 );   
    }


void bitmap_draw_raw( render_t* render, bitmap_t* bmp, float x, float y, float w, float h, uint32_t color )
    {
    y -= (int)render->yoffset;
    if( w <= 0 || h <= 0 ) return;
    float ix = ( ( x / render->logical_width ) * render->view_width );
    float iy = ( ( y / render->logical_height ) * render->view_height );
    float iw = ( ( w / render->logical_width ) * render->view_width );
    float ih = ( ( h / render->logical_height ) * render->view_height );
	float x1 = ix / render->view_width;
    float y1 = iy / render->view_height;
    float x2 = x1 + iw / render->view_width;
    float y2 = y1 + ih / render->view_height;

	GLfloat vertices[] = 
		{ 
		2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		};
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, bmp->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, 0 );   
    }


float bitmap_draw_tab( render_t* render, bitmap_t* shadow, bitmap_t* highlight, float x, float y, float h, uint32_t color )
    {
    y -= (int)render->yoffset;
    float w = ( h * shadow->width ) / shadow->height;
	float x1 = x / render->logical_width;
    float y1 = y / render->logical_height;
    float x2 = ( x + w ) / render->logical_width;
    float y2 = ( y + h ) / render->logical_height;

	GLfloat vertices[] = 
		{ 
		2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		};
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, shadow->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, highlight->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, 0 );   
    return w;
    }


float bitmap_draw_tab_tiled( render_t* render, bitmap_t* shadow, bitmap_t* highlight, float x, float y, float h, float w, uint32_t color )
    {
    y -= (int)render->yoffset;
	float x1 = x / render->logical_width;
    float y1 = y / render->logical_height;
    float x2 = ( x + w ) / render->logical_width;
    float y2 = ( y + h ) / render->logical_height;

	GLfloat vertices[] = 
		{ 
		2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		};
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, shadow->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, highlight->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, 0 );   
    return w;
    }


void bitmap_draw_tiled( render_t* render, bitmap_t* bmp )
    {
    float scrw = render->view_width;
    float scrh = render->view_height;
    int hcount = (int)( scrw / bmp->width ) + 1;
    int vcount = (int)( scrh / bmp->height ) + 1;
    
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), 1.0f, 1.0f, 1.0f, 1.0f );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, bmp->tex.handle );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    for( int y = 0; y < vcount; ++y )
        {
        for( int x = 0; x < hcount; ++x )
            {
	        float x1 = x * ( bmp->width / scrw );
            float y1 = y * ( bmp->height / scrh );
            float x2 = ( x + 1 ) * ( bmp->width / scrw );
            float y2 = ( y + 1 ) * ( bmp->height / scrh );

	        GLfloat vertices[] = 
		        { 
		        2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f, 0.0f,
		        2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		        2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		        2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		        };
	        glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );

	        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
            }
        }
	glBindTexture( GL_TEXTURE_2D, 0 );   
    }

typedef struct glyph_t
    {
    int glyph_index;
    int w;
    int h;
    int x;
    int y;
    int advance;
    } glyph_t;

typedef struct font_t
    {
    file_t* file;
    stbtt_fontinfo info;
    int point_size;
    int line_spacing;
    int baseline;
    int height;
    glyph_t glyphs[ 256 ];
    uint8_t* glyph_buffer;
    int glyph_buffer_capacity;
    char filename[ 256 ];
    } font_t;


font_t* font_create( render_t* render, char const* filename, int point_size )
    {
    (void) render;
    font_t* font = (font_t*) malloc( sizeof( font_t ) );
    memset( font, 0, sizeof( *font ) );

    font->point_size = point_size;

    font->file = file_load( filename, FILE_MODE_BINARY, 0 );
    if( !font->file ) 
        {
        free( font );
        return 0;
        }
    
    stbtt_InitFont( &font->info, (unsigned char*) font->file->data, stbtt_GetFontOffsetForIndex( (unsigned char*) font->file->data, 0 ) );

    int ascent = 0;
    int descent = 0;
    int linegap = 0;
    stbtt_GetFontVMetrics( &font->info, &ascent, &descent, &linegap );
    font->baseline = ascent;
    font->line_spacing = linegap;
    font->height = ascent - descent;

    int max_w = 0;
    int max_h = 0;
    for( int i = 0; i < 256; ++i )
        {
        int codepoint = (uint8_t) i;
        int glyph_index = stbtt_FindGlyphIndex( &font->info, codepoint );

        int advance = 0;
        int left_side_bearing = 0;
        stbtt_GetGlyphHMetrics( &font->info, glyph_index, &advance, &left_side_bearing );
        font->glyphs[ i ].advance = advance;

        int x0 = 0;
        int y0 = 0;
        int x1 = 0;
        int y1 = 0;

        stbtt_GetGlyphBitmapBoxSubpixel( &font->info, glyph_index, 1.0f, 1.0f, 0.0f, 0.0f, &x0, &y0, &x1, &y1 );
        if( max_w < x1 - x0 ) max_w = (int)( x1 - x0 );
        if( max_h < y1 - y0 ) max_h = (int)( y1 - y0 );

        font->glyphs[ i ].glyph_index = glyph_index;
        font->glyphs[ i ].x = x0;
        font->glyphs[ i ].y = y0;
        font->glyphs[ i ].w = x1 - x0;
        font->glyphs[ i ].h = y1 - y0;
        }
    max_w++;
    max_h++;
    font->glyph_buffer_capacity = max_w * max_h;
    font->glyph_buffer = (uint8_t*) malloc( font->glyph_buffer_capacity * sizeof( uint8_t ) );
    strcpy( font->filename, filename );    
    return font;
    }


void font_destroy( render_t* render, font_t* font )
    {
    if( !font ) return;
    (void) render, font; 
    free( font->glyph_buffer );
    file_destroy( font->file );
    free( font );
    }


typedef struct text_t
    {
    char string[ 256 ];
    float baseline;
    float w_acc;
    int width;
    int height;
    int tex_width;
    int tex_height;
    tex_t tex;
    } text_t;


int text_width( render_t* render, font_t* font, char const* string )
    {
    float scr_scale = render->view_height / render->logical_height;
    if( scr_scale <= 0.0f ) scr_scale  = FLT_EPSILON;
    float scale = stbtt_ScaleForPixelHeight( &font->info, (float) font->point_size * scr_scale );
    if( scale <= 0.0f ) scale  = FLT_EPSILON;

    int w_acc = 0;
    int len = (int) strlen( string );
    for( int i = 0; i < len; ++i )
        {
        glyph_t* glyph = &font->glyphs[ (uint8_t) string[ i ] ];
        w_acc += glyph->advance;
        }
    int w = ( (int) ( ( w_acc * scale ) / scr_scale ) );
    return w;
    }


float text_widthf( render_t* render, font_t* font, char const* string )
    {
    float scr_scale = render->view_height / render->logical_height;
    if( scr_scale <= 0.0f ) scr_scale  = FLT_EPSILON;
    float scale = stbtt_ScaleForPixelHeight( &font->info, (float) font->point_size * scr_scale );
    if( scale <= 0.0f ) scale  = FLT_EPSILON;

    int w_acc = 0;
    int len = (int) strlen( string );
    for( int i = 0; i < len; ++i )
        {
        glyph_t* glyph = &font->glyphs[ (uint8_t) string[ i ] ];
        w_acc += glyph->advance;
        }
    float w = ( ( w_acc * scale ) / scr_scale );
    return w;
    }


text_t* text_create( render_t* render, font_t* font, char const* string )
    {
    (void) render, font, string;
    text_t* text = (text_t*) malloc( sizeof( text_t ) );
    memset( text, 0, sizeof( *text ) );

    float scr_scale = render->view_height / render->logical_height;
    if( scr_scale <= 0.0f ) scr_scale  = FLT_EPSILON;
    float scale = stbtt_ScaleForPixelHeight( &font->info, (float) font->point_size * scr_scale );
    if( scale <= 0.0f ) scale  = FLT_EPSILON;

    int w_acc = 0;
    int len = (int) strlen( string );
    for( int i = 0; i < len; ++i )
        {
        glyph_t* glyph = &font->glyphs[ (uint8_t) string[ i ] ];
        w_acc += glyph->advance;
        }
    
    int w = ( (int) ( w_acc * scale ));
    int h = ( (int)( font->height * scale ) );

    int tw = (int)pow2_ceil( (unsigned int) w + 8 );
    int th = (int)pow2_ceil( (unsigned int) h + 8 );
    uint32_t* bitmap = (uint32_t*) malloc( tw * th * sizeof( uint32_t ) );
    memset( bitmap, 0, tw * th * sizeof( uint32_t ) );
    float xpos = 0;
    for( int i = 0; i < len; ++i )
        {
        int codepoint = (uint8_t) string[ i ];
        glyph_t* glyph = &font->glyphs[ codepoint ];
        int glyph_index = glyph->glyph_index;
        int x0 = 0;
        int y0 = 0;
        int x1 = 0;
        int y1 = 0;

        float x_shift = xpos - floorf( xpos );      
        
        stbtt_GetGlyphBitmapBoxSubpixel( &font->info, glyph_index, scale, scale, x_shift, 0.0f, &x0, &y0, &x1, &y1 );
        int glyph_w = x1 - x0;
        int glyph_h = y1 - y0;
        if( font->glyph_buffer_capacity < glyph_w * glyph_h )
            {
            font->glyph_buffer_capacity = glyph_w * glyph_h;
            font->glyph_buffer = (uint8_t*)realloc( font->glyph_buffer, sizeof( uint8_t ) * font->glyph_buffer_capacity );
            }

        stbtt_MakeGlyphBitmapSubpixel( &font->info, font->glyph_buffer, glyph_w, glyph_h, glyph_w, scale, scale, x_shift, 0.0f, glyph_index );
        for( int y = 0; y < glyph_h; ++y )
            {
            for( int x = 0; x < glyph_w; ++x )
                {
                uint32_t val = (uint32_t) font->glyph_buffer[ x + y * glyph_w ];
                int offset = 4 + (int)xpos + x0 + x;
                offset += ( 4 + y + y0 + (int)( font->baseline * scale ) ) * tw;
                uint32_t* pixel = &bitmap[ offset ];
                uint32_t current = (*pixel) >> 24;
                val = val + current;
                val = val > 255 ? 255 : val;
                *pixel = ( val << 24 ) | 0x00ffffff;
                }
            }
        int glyph_index_2 = font->glyphs[ (unsigned char)( string[ i + 1 ] ) ].glyph_index;
        int kern = stbtt_GetGlyphKernAdvance( &font->info, glyph_index, glyph_index_2 );
        if( kern != 0 )
            kern = kern;
        xpos += ( kern + glyph->advance ) * scale;
        }

    // checkerboard test texture
    //for( int y = 0; y < h; ++y ) for( int x = 0; x < w; ++x ) text->bitmap[ x + y * w ] = ( x + y ) & 1  ? 0xffffffff : 0xff000000;


    text->tex = render_internal_allocate_texture( render, tw, th, false );
    glActiveTexture( GL_TEXTURE0 ); 
    glBindTexture( GL_TEXTURE_2D, text->tex.handle );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, tw, th, GL_RGBA, GL_UNSIGNED_BYTE, bitmap ); 

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    #define RENDER_GL_CLAMP_TO_BORDER 0x812D
    #define RENDER_GL_TEXTURE_BORDER_COLOR 0x1004
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, RENDER_GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, RENDER_GL_CLAMP_TO_BORDER );
	float color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv( GL_TEXTURE_2D, RENDER_GL_TEXTURE_BORDER_COLOR, color );	
    #undef RENDER_GL_CLAMP_TO_BORDER
    #undef RENDER_GL_TEXTURE_BORDER_COLOR

    glBindTexture( GL_TEXTURE_2D, 0 );   

    text->baseline = ( font->baseline * scale ) / scr_scale;
    text->w_acc = ( ( w_acc * scale ) / scr_scale );
    text->width = (int)( w / scr_scale );
    text->height = (int)( h / scr_scale );
    text->tex_width = tw;
    text->tex_height = th;
    strcpy( text->string, string );    
    free( bitmap );
    return text;
    }


void text_destroy( render_t* render, text_t* text )
    {
    if( !text ) return;
    (void) render, text;    
    render_internal_free_texture( render, text->tex );
    free( text );
    }


void text_draw( render_t* render, text_t* text, int x, int y, uint32_t color )
    {
    y -= (int)render->yoffset;
    float scale = render->view_height / render->logical_height;
    int ix = (int)( ( ( x - 4 / scale ) / render->logical_width ) * render->view_width );
    int iy = (int)( ( ( y - text->baseline - 4 / scale ) / render->logical_height ) * render->view_height );
	float x1 = ix / render->view_width;
    float y1 = iy / render->view_height;
    float x2 = x1 + text->tex_width / render->view_width;
    float y2 = y1 + text->tex_height / render->view_height;

	GLfloat vertices[] = 
		{ 
		2.0f * x1 - 1.0f, -2.0f * y1 + 1.0f, 0.0f , 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y1 + 1.0f, 1.0f, 0.0f,
		2.0f * x2 - 1.0f, -2.0f * y2 + 1.0f, 1.0f, 1.0f,
		2.0f * x1 - 1.0f, -2.0f * y2 + 1.0f, 0.0f, 1.0f,
		};
	glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( GLfloat ), vertices, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, render->vertexbuffer );

    float a = ( ( color >> 24 ) & 0xff ) / 255.0f;
    float r = ( ( color >> 16 ) & 0xff ) / 255.0f;
    float g = ( ( color >> 8  ) & 0xff ) / 255.0f;
    float b = ( ( color       ) & 0xff ) / 255.0f;
	glUseProgram( render->shader );
	glUniform4f( glGetUniformLocation( render->shader, "col" ), r, g, b, a );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, text->tex.handle );
	glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
	glBindTexture( GL_TEXTURE_2D, 0 );   
    }



// text edit 

#define STB_TEXTEDIT_CHARTYPE char
#include "libs/stb_textedit.h"

#define EDIT_CTRL ( 1 << 29 )
#define EDIT_KEYDOWN ( 1 << 28 )

typedef struct edit_t
    {
    render_t* render;
    font_t* font;
    char* str;
    int str_length;
    int str_capacity;

    float x;
    float y;
    text_t* text;
    bool cursor;
    int cursor_tick;
    bool dragging;
    
    STB_TexteditState state;
    } edit_t;

void edit_internal_layout_row( StbTexteditRow* row, edit_t* str, int start_pos );
float edit_internal_get_width( edit_t* str, int start_pos, int char_pos );
int edit_internal_key_to_text( int k );
void edit_internal_deletechars( edit_t* str, int start_pos, int length );
int edit_internal_insertchars(  edit_t* str, int insert_pos, char const* characters, int length );   

#define STB_TEXTEDIT_STRING edit_t
#define STB_TEXTEDIT_STRINGLEN( obj ) ( (obj)->str_length )
#define STB_TEXTEDIT_LAYOUTROW( r, obj, n ) ( edit_internal_layout_row( (r), (obj) , (n) ) )
#define STB_TEXTEDIT_GETWIDTH( obj, n, i) ( edit_internal_get_width( (obj), (n), (i) ) )
#define STB_TEXTEDIT_KEYTOTEXT( k ) ( edit_internal_key_to_text( k ) )
#define STB_TEXTEDIT_GETCHAR( obj, i ) ( (obj)->str[ (i) ] )
#define STB_TEXTEDIT_NEWLINE '\n'             
#define STB_TEXTEDIT_DELETECHARS( obj, i, n ) ( edit_internal_deletechars( (obj), (i), (n) ) )
#define STB_TEXTEDIT_INSERTCHARS( obj, i, c, n ) ( edit_internal_insertchars( (obj), (i), (c), (n) ) )
#define STB_TEXTEDIT_K_SHIFT ( 1 << 30 )
#define STB_TEXTEDIT_K_LEFT (APP_KEY_LEFT | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_RIGHT (APP_KEY_RIGHT | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_UP (APP_KEY_UP | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_DOWN (APP_KEY_DOWN | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_LINESTART (APP_KEY_HOME | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_LINEEND (APP_KEY_END | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_TEXTSTART (APP_KEY_HOME | EDIT_CTRL | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_TEXTEND (APP_KEY_END | EDIT_CTRL | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_DELETE (APP_KEY_DELETE | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_BACKSPACE (APP_KEY_BACK | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_UNDO (APP_KEY_Z | EDIT_CTRL | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_REDO (APP_KEY_Y | EDIT_CTRL | EDIT_KEYDOWN )

#define STB_TEXTEDIT_K_INSERT (APP_KEY_INSERT | EDIT_KEYDOWN )
#define STB_TEXTEDIT_IS_SPACE(ch)  isspace(ch)
#define STB_TEXTEDIT_K_WORDLEFT (APP_KEY_LEFT | EDIT_CTRL | EDIT_KEYDOWN )
#define STB_TEXTEDIT_K_WORDRIGHT (APP_KEY_RIGHT | EDIT_CTRL | EDIT_KEYDOWN )

#define STB_TEXTEDIT_IMPLEMENTATION
#pragma warning( disable: 4365 )
#include "libs/stb_textedit.h"

edit_t* edit_create( render_t* render, font_t* font )
    {    
    edit_t* edit = (edit_t*) malloc( sizeof( edit_t ) );
    memset( edit, 0, sizeof( *edit ) );
    edit->render = render;
    edit->font = font;
    edit->str_length = 0;
    edit->str_capacity = 256;
    edit->str = (char*) malloc( edit->str_capacity * sizeof( char ) );

    strcpy( edit->str, "Search" );
    edit->str_length = (int) strlen( edit->str );
    edit->text = 0;

    stb_textedit_initialize_state( &edit->state, 1 );
    edit->state.cursor = edit->str_length;
    edit->state.cursor_at_end_of_line = 1;
    return edit;
    }


void edit_destroy( render_t* render, edit_t* edit )
    {
    if( !edit ) return;
    if( edit->text ) text_destroy( render, edit->text );
    free( edit->str );
    free( edit );
    }


bool edit_update( edit_t* edit, app_input_t input, bool ctrl, bool shift, int mouse_x, int mouse_y )
    {
    bool redraw = false;
    STB_TexteditState prev_state = edit->state;
    
    float x = (float) mouse_x - edit->x; 
    float y = (float) mouse_y - edit->y; 
    for( int i = 0; i < input.count; ++i )
        {
        if( input.events[ i ].type == APP_INPUT_DOUBLE_CLICK )
            {
            if( input.events[ i ].data.key == APP_KEY_LBUTTON )
                {
                stb_textedit_click( edit, &edit->state, x, y );
                int start = edit->state.cursor;
                int end = edit->state.cursor;
                while( start > 0 && !STB_TEXTEDIT_IS_SPACE( (uint8_t) edit->str[ start ] ) ) --start;
                while( start < edit->str_length && STB_TEXTEDIT_IS_SPACE( (uint8_t) edit->str[ start ] ) ) ++start;
                while( end < edit->str_length && !STB_TEXTEDIT_IS_SPACE( (uint8_t) edit->str[ end ] ) ) ++end;
                if( end - start > 0 )
                    {
                    edit->state.select_start = start;
                    edit->state.select_end = end;
                    edit->dragging = false;
                    }
                }
            }
        else if( input.events[ i ].type == APP_INPUT_MOUSE_MOVE && edit->dragging )
            {
            stb_textedit_drag( edit, &edit->state, x, y );
            }
        else if( input.events[ i ].type == APP_INPUT_KEY_DOWN )
            {
            if( input.events[ i ].data.key == APP_KEY_LBUTTON )
                {
                stb_textedit_click( edit, &edit->state, x, y );
                edit->dragging = true;
                }
            else if( 
                ( ctrl && input.events[ i ].data.key == APP_KEY_C ) || 
                ( ctrl && input.events[ i ].data.key == APP_KEY_X ) || 
                ( ctrl && input.events[ i ].data.key == APP_KEY_INSERT ) ||
                ( shift && input.events[ i ].data.key == APP_KEY_DELETE ) )
                {
                int start = edit->state.select_start;
                int end = edit->state.select_end;
                if( start > end ) 
                    {
                    int temp = start;
                    start = end;
                    end = temp;
                    }
                if( end - start > 0 )
                    {
                    char saved = edit->str[ end ];
                    edit->str[ end ] = 0;
                    copy_to_clipboard( edit->str + start );
                    edit->str[ end ] = saved;
                    }
                if( ( ctrl && input.events[ i ].data.key == APP_KEY_X ) || ( shift && input.events[ i ].data.key == APP_KEY_DELETE ) )
                    stb_textedit_cut( edit, &edit->state );
                }
            else if( 
                ( ctrl && input.events[ i ].data.key == APP_KEY_V ) || 
                ( shift && input.events[ i ].data.key == APP_KEY_INSERT ) )
                {
                int len = copy_from_clipboard( 0, 0 );
                char* str = (char*) malloc( sizeof( char ) * ( len + 1 ) );
                len = copy_from_clipboard( str, len + 1 );
                stb_textedit_paste( edit, &edit->state, str, len );
                free( str );
                }
            else if( ctrl && input.events[ i ].data.key == APP_KEY_A )
                {
                edit->state.select_start = 0;
                edit->state.select_end = edit->str_length;
                }
            else
                {
                int key = ( (int) input.events[ i ].data.key ) | EDIT_KEYDOWN;
                if( shift ) key |= STB_TEXTEDIT_K_SHIFT;
                if( ctrl ) key |= EDIT_CTRL;
                stb_textedit_key( edit, &edit->state, key );           
                }
            }
        else if( input.events[ i ].type == APP_INPUT_KEY_UP )
            {
            if( input.events[ i ].data.key == APP_KEY_LBUTTON && edit->dragging )
                {
                stb_textedit_drag( edit, &edit->state, x, y );
                edit->dragging = false; 
                }
            }
        else if( input.events[ i ].type == APP_INPUT_CHAR )
            {
            uint8_t c = (uint8_t) input.events[ i ].data.char_code;
            if( c >= ' ' )
                stb_textedit_key( edit, &edit->state, (int) c );           
            }
        }

    int cursor_rate = 25;
    if( ( ( edit->cursor_tick / cursor_rate ) ) != ( ( ( edit->cursor_tick + 1 ) / cursor_rate ) ) ) 
        {
        edit->cursor = !edit->cursor;
        redraw = true;
        }
    edit->cursor_tick++;

    if( edit->text == 0 ) { edit->cursor = true; redraw = true; }
    if( memcmp( &prev_state, &edit->state, sizeof( prev_state ) ) != 0 ) { edit->cursor = true; redraw = true; };
    return redraw;
    }


void edit_draw( render_t* render, edit_t* edit, uint32_t color )
    {
    if( !edit->text ) edit->text = text_create( render, edit->font, edit->str );  

    int start = edit->state.select_start;
    int end = edit->state.select_end;
    if( start > end ) 
        {
        int temp = start;
        start = end;
        end = temp;
        }
    if( end - start > 0 )
        {
        char saved = edit->str[ start ];
        edit->str[ start ] = 0;
        float sel_start = text_widthf( render, edit->font, edit->str );
        edit->str[ start ] = saved;

        saved = edit->str[ end ];
        edit->str[ end ] = 0;
        float sel_end = text_widthf( render, edit->font, edit->str );
        edit->str[ end ] = saved;

        render_draw_rectf( render, ( edit->x + sel_start ), (float)( edit->y - edit->text->baseline ) - 0.5f , sel_end - sel_start, (float)( edit->text->height ) - 0.5f , 0xffd98b1a );
        }

    text_draw( render, edit->text, (int)edit->x, (int)edit->y, color );    

    if( edit->cursor && end - start <= 0 )
        {
        float curs_x = 0.0f;
        if( edit->state.cursor >= 0 )
            {
            char saved = edit->str[ edit->state.cursor ];
            edit->str[ edit->state.cursor ] = 0;
            curs_x = text_widthf( render, edit->font, edit->str );
            edit->str[ edit->state.cursor ] = saved;
            }

        render_draw_rect( render, (int)( edit->x + curs_x ), (int) ( edit->y - edit->text->baseline ), 2, (int)( edit->text->height), 0xff736d64 );
        }

    }


// returns the results of laying out a line of characters starting from character #n (see discussion below)
void edit_internal_layout_row( StbTexteditRow* row, edit_t* edit, int start_pos )
    {
    if( !edit->text ) return;
    (void) start_pos;
    char saved = edit->str[ start_pos ];
    edit->str[ start_pos ] = 0;
    row->x0 = text_widthf( edit->render, edit->font, edit->str );
    edit->str[ start_pos ] = saved;
    row->x1 = text_widthf( edit->render, edit->font, edit->str + start_pos  );
    row->baseline_y_delta = (float)edit->text->height;
    row->ymin = -(float)edit->text->baseline;
    row->ymax = row->ymin + (float)edit->text->height;
    row->num_chars = edit->str_length - start_pos;
    }


// returns the pixel delta from the xpos of the i'th character to the xpos of the i+1'th char for a line of characters
// starting at character #n (i.e. accounts for kerning with previous char)
float edit_internal_get_width( edit_t* edit, int start_pos, int char_pos )
    {
    (void) start_pos;
    char saved = edit->str[ char_pos + 1 ];
    edit->str[ char_pos + 1 ] = 0;
    float delta = text_widthf( edit->render, edit->font, edit->str + char_pos );
    edit->str[ char_pos + 1 ] = saved;

    return delta;
    }


// maps a keyboard input to an insertable character (return type is int, -1 means not valid to insert)
int edit_internal_key_to_text( int k )
    {
    if( ( k & EDIT_KEYDOWN ) == 0 )
        return k;

    return -1;
    }


// delete n characters starting at i
void edit_internal_deletechars( edit_t* str, int start_pos, int length )
    {
    (void) str, start_pos, length;
    if( str->str_length - ( start_pos + length ) > 0 )
        memmove( str->str + start_pos, str->str + start_pos + length, ( str->str_length - ( start_pos + length ) ) * sizeof( char ) );
    str->str_length -= length;
    str->str[ str->str_length ] = 0;
    if( str->text ) text_destroy( str->render, str->text );
    str->text = 0;
    }


// insert n characters at i (pointed to by STB_TEXTEDIT_CHARTYPE*)
int edit_internal_insertchars(  edit_t* str, int insert_pos, char const* characters, int length )   
    {
    (void) str, insert_pos, characters, length;
    if( str->str_length + length + 1 >= str->str_capacity )
        {
        str->str_capacity = (int) pow2_ceil( (uint32_t) ( str->str_length + length + 1 ) );
        str->str = (char*) realloc( str->str, str->str_capacity * sizeof( char ) );
        }
    if( str->str_length - length > 0 )
        memmove( str->str + insert_pos + length, str->str + insert_pos, ( str->str_length - insert_pos ) * sizeof( char ) );
    memcpy( str->str + insert_pos, characters, length * sizeof( char ) );
    str->str_length += length;
    str->str[ str->str_length ] = 0;
    if( str->text ) text_destroy( str->render, str->text );
    str->text = 0;
    return 1;
    }


typedef struct button_t
    {
    render_t* render;
	bitmap_t* bmp;
    int x, y, w, h;
    bool override_bounds;
    int bx, by, bw, bh;
    float rotation;

	bool hover;
    } button_t;


void button_init( button_t* button, render_t* render, bitmap_t* bitmap )
    {
    memset( button, 0, sizeof( *button ) );
    button->render = render;
    button->bmp = bitmap;
    button->w = bitmap->width;
    button->h = bitmap->height;
    button->rotation = 0.0f;
    }


void button_term( button_t* button )
    {
    if( !button ) return;
    if( button->bmp ) bitmap_destroy( button->render, button->bmp );
    }


void button_override_bounds( button_t* button, int x, int y, int w, int h )
    {
    button->override_bounds = true;
    button->bx = x;
    button->by = y;
    button->bw = w;
    button->bh = h;
    }


void button_modify_bounds( button_t* button, int x1, int y1, int x2, int y2 )
    {
    button->override_bounds = true;
    button->bx = button->x + x1;
    button->by = button->y + y1;
    button->bw = button->w - x1 + x2;
    button->bh = button->h - y1 + y2;
    }


void button_size( button_t* button, int x, int y, int w, int h )
    {
    button->x = x;
    button->y = y;
    button->w = w;
    button->h = h;
    }


void button_rotation( button_t* button, float radians ) 
    {
    button->rotation = radians;
    }


void button_rotation_off( button_t* button ) 
    {
    button->rotation = 0.0f;
    }


bool button_clicked( button_t* button, int mouse_x, int mouse_y, bool clicked ) 
    {
    bool hover = false;
    if( button->override_bounds )
        hover = mouse_x >= button->bx && mouse_x < button->bx + button->bw && mouse_y >= button->by && mouse_y < button->by + button->bh;
    else
        hover = mouse_x >= button->x && mouse_x < button->x + button->w && mouse_y >= button->y && mouse_y < button->y + button->h;
    return hover && clicked;
    }


bool button_update( button_t* button, int mouse_x, int mouse_y, bool* out_hover )
    {
    bool hover = false;
    if( button->override_bounds )
        hover = mouse_x >= button->bx && mouse_x < button->bx + button->bw && mouse_y >= button->by && mouse_y < button->by + button->bh;
    else
        hover = mouse_x >= button->x && mouse_x < button->x + button->w && mouse_y >= button->y && mouse_y < button->y + button->h;

    bool redraw = false;
	if( hover )
		{
		if( !button->hover ) redraw = true;
		button->hover = true;
	    }
	else
		{
		if( button->hover ) redraw = true;
		button->hover = false;
		}

    if( out_hover ) *out_hover = button->hover;
    return redraw;
    }


void button_draw( button_t* button, uint32_t color, uint32_t highlight )
    {
    if( button->rotation == 0.0f )
        {
        bitmap_draw( button->render, button->bmp, button->x, button->y, button->w, button->h, button->hover ? highlight : color );    
        }
    else
        {
        bitmap_draw_rotated( button->render, button->bmp, button->x, button->y, button->w, button->h, button->rotation, button->hover ? highlight : color );    
        }
    }


typedef struct slider_bitmaps_t
    {
	bitmap_t* left;
    bitmap_t* middle;
    bitmap_t* right;
    bitmap_t* gap;
    bitmap_t* handle;
    } slider_bitmaps_t;


typedef struct slider_t
    {
    render_t* render;
	slider_bitmaps_t bmp;
    int x, y, w, h;

    bool handle_hover;
    bool prev_lbutton;
    bool dragging;
    float drag_offset;
    float drag_value;
    float value;
    } slider_t;


void slider_init( slider_t* slider, render_t* render, slider_bitmaps_t bitmaps, int x, int y, int w, int h )
    {
    memset( slider, 0, sizeof( *slider ) );
    slider->render = render;
    slider->bmp = bitmaps;
    slider->x = x;
    slider->y = y;
    slider->w = w;
    slider->h = h;
    }


void slider_term( slider_t* slider )
    {
    (void) slider;
    }



void slider_size( slider_t* slider, int x, int y, int w, int h )
    {
    slider->x = x;
    slider->y = y;
    slider->w = w;
    slider->h = h;
    }


void slider_value_set( slider_t* slider, float value ) 
    {
    slider->value = value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : isnan( value ) ? 0.0f : value;
    }


float slider_value( slider_t* slider ) 
    {
    return slider->dragging ? slider->drag_value : slider->value;
    }


bool slider_is_dragging( slider_t* slider ) 
    {
    return slider->dragging;
    }


bool slider_update( slider_t* slider, int mouse_x, int mouse_y, bool lbutton, bool* out_changed )
    {
    if( out_changed ) *out_changed = false;
    float x = (float) slider->x;
    float w = (float) slider->w;
    float size = (float) slider->h;   
    float left_width = ( slider->bmp.left->width / (float) slider->bmp.left->height ) * size;
    float right_width = ( slider->bmp.right->width / (float) slider->bmp.right->height ) * size;
    float handle_width = ( slider->bmp.handle->width / (float) slider->bmp.handle->height ) * size;
    float handle_pos = x + ( w - handle_width - left_width ) * slider->value + left_width; 

    float pad = slider->h / 2.0f;

    bool handle_hover = false;
    handle_hover = mouse_x >= handle_pos - pad && mouse_x < handle_pos + handle_width + pad && mouse_y >= slider->y - pad && mouse_y < slider->y + slider->h + pad;

    bool slider_hover = false;
    slider_hover = mouse_x >= x && mouse_x < x + w && mouse_y >= slider->y - pad && mouse_y < slider->y + slider->h + pad;

    bool redraw = false;

    if( slider->dragging )
        {
		redraw = true;
        float value = ( mouse_x - x - slider->drag_offset - left_width ) / (float) ( w - left_width - right_width - handle_width );
        value = value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : isnan( value ) ? 0.0f : value;
        slider->drag_value = value;
        if( !lbutton ) 
            {
            slider->dragging = false;
            slider->handle_hover = true;
            slider->value = slider->drag_value;
            if( out_changed ) *out_changed = true;
    		redraw = true;
            }        
        }
    else 
        {
        if( slider->handle_hover && lbutton && !slider->prev_lbutton )
            {
            slider->drag_value = slider->value;
            slider->dragging = true;
            slider->drag_offset = mouse_x - handle_pos;
            if( slider->drag_offset > handle_width + pad ) slider->drag_offset = handle_width;
            if( slider->drag_offset < -( handle_width + pad )) slider->drag_offset = -( handle_width + pad);
            redraw = true;
            }
        else if( slider_hover && lbutton && !slider->prev_lbutton )
            {
            handle_pos = (float)mouse_x - ( handle_width / 2.0f );
            float value = ( handle_pos - left_width - x ) / ( w - handle_width - left_width );
            value = value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : isnan( value ) ? 0.0f : value;
            slider->drag_value = value;
            slider->dragging = true;
            slider->drag_offset = mouse_x - handle_pos;
            if( slider->drag_offset > handle_width + pad ) slider->drag_offset = handle_width;
            if( slider->drag_offset < -( handle_width + pad )) slider->drag_offset = -( handle_width + pad);
            redraw = true;
            }

	    if( handle_hover && !slider->handle_hover )
		    {
            if( !lbutton ) 
                {
		        redraw = true;
		        slider->handle_hover = true;
                }
	        }
	    else if( !handle_hover && slider->handle_hover)
		    {
		    redraw = true;
		    slider->handle_hover = false;
		    }
        }

    slider->prev_lbutton = lbutton;

    return redraw;
    }


void slider_draw( slider_t* slider, uint32_t color, uint32_t highlight )
    {
    float value = slider->dragging ? slider->drag_value : slider->value;

    float x = (float) slider->x;
    float y = (float) slider->y;
    float w = (float) slider->w;
    float size = (float) slider->h;
    
    float left_width = ( slider->bmp.left->width / (float) slider->bmp.left->height ) * size;
    float right_width = ( slider->bmp.right->width / (float) slider->bmp.right->height ) * size;
    float handle_width = ( slider->bmp.handle->width / (float) slider->bmp.handle->height ) * size;
    float handle_pos = ( w - left_width - right_width - handle_width ) * value + left_width; 
    float middle_width1 = handle_pos - left_width;
    float middle_width2 = ( w - handle_pos - handle_width - right_width );

    bitmap_draw_raw( slider->render, 
        slider->bmp.left, 
        x, 
        y, 
        left_width, 
        size,
        color );    

    bitmap_draw_raw( slider->render, 
        slider->bmp.middle, 
        x + left_width, 
        y, 
        middle_width1, 
        size,
        color );    

    bitmap_draw_raw( slider->render, 
        slider->bmp.middle, 
        x + handle_pos + handle_width, 
        y, 
        middle_width2, 
        size,
        color );    

    bitmap_draw_raw( slider->render, 
        slider->bmp.right, 
        x + w - right_width, 
        y, 
        right_width, 
        size,
        color );    

    bitmap_draw_raw( slider->render, 
        slider->bmp.gap, 
        x + handle_pos, 
        y, 
        handle_width, 
        size,
        color );    

    bitmap_draw_raw( slider->render, 
        slider->bmp.handle, 
        x + handle_pos, 
        y, 
        handle_width, 
        size,
        slider->handle_hover || slider->dragging ? highlight : color );    

    }
