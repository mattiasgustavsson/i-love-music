
void image_merge( uint32_t* canvas, int canvas_width, int canvas_height, uint32_t* temp, int temp_width, int temp_height, int xp, int yp )
    {
    for( int y = 0; y < temp_height; ++y )
        {
        for( int x = 0; x < temp_width; ++x )
            {
            if( x + xp < 0 || x + xp >= canvas_width || y + yp < 0 || y + yp >= canvas_height )
                continue;

            uint32_t c = canvas[ ( x + xp ) + ( y + yp ) * canvas_width ];
		    int cr = (int)( ( c & 0x000000ff ) );
		    int cg = (int)( ( c & 0x0000ff00 ) >> 8 );
		    int cb = (int)( ( c & 0x00ff0000 ) >> 16 );                
		    int ca = (int)( ( c & 0xff000000 ) >> 24 );                
            uint32_t t = temp[ x + y * temp_width ];
		    int tr = (int)( ( t & 0x000000ff ) );
		    int tg = (int)( ( t & 0x0000ff00 ) >> 8 );
		    int tb = (int)( ( t & 0x00ff0000 ) >> 16 );                
		    int ta = (int)( ( t & 0xff000000 ) >> 24 );                

            int a = ta + ( ( ca * ( 255 - ta ) ) >> 8 );
            if( a > 255 ) a = 255;
            int r = a == 0 ? 0 : ( ( tr * ta + ( cr * ca * ( 255 - ta ) ) / a ) ) >> 8;
            int g = a == 0 ? 0 : ( ( tg * ta + ( cg * ca * ( 255 - ta ) ) / a ) ) >> 8;
            int b = a == 0 ? 0 : ( ( tb * ta + ( cb * ca * ( 255 - ta ) ) / a ) ) >> 8;
            c = (uint32_t)( ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r );
            canvas[ ( x + xp ) + ( y + yp ) * canvas_width ] = c;
            }
        }
    }


inline mat44_t look_at_lh( vec3_t eye, vec3_t at, vec3_t up ) { vec3_t at_vec = vec3_normalize( vec3_sub( at, eye ) ); vec3_t right_vec = vec3_normalize( vec3_cross( up, at_vec ) ); vec3_t up_vec = vec3_cross( at_vec, right_vec ); return mat44( vec4v3f( right_vec, 0.0f ), vec4v3f( up_vec, 0.0f ), vec4v3f( at_vec, 0.0f ), vec4v3f( eye, 1.0f ) ); }
inline mat44_t perspective_lh( float w, float h, float zn, float zf ) 
    { 
    return mat44( 
        vec4( 2.0f * zn / w, 0.0f, 0.0f, 0.0f ), 
        vec4( 0.0f, 2.0f * zn / h, 0.0f, 0.0f ), 
        vec4( 0.0f, 0.0f, zf / ( zf - zn ), 1.0f ), 
        vec4( 0.0f, 0.0f, zn * zf / ( zn - zf ), 0.0f ) ); }
inline mat44_t perspective_fov_lh( float fovy, float aspect, float zn, float zf ) 
	{ 
	float h = 1.0f / internal_vecmath_tan( fovy * 0.5f ); 
	float w = h / aspect; 
	return mat44( 
		vec4( w, 0.0f, 0.0f, 0.0f ), 
		vec4( 0.0f, h, 0.0f, 0.0f ), 
		vec4( 0.0f, 0.0f, zf / ( zf - zn ), 1.0f ), 
		vec4( 0.0f, 0.0f, -zn * zf / ( zf - zn ), 0.0f ) 
		); 
	}
inline mat44_t rotation_x( float angle ) { float s = internal_vecmath_sin( angle ); float c = internal_vecmath_cos( angle ); return mat44( vec4( 1.0f, 0.0f, 0.0f, 0.0f ), vec4( 0.0f, c, s, 0.0f ), vec4( 0.0f, -s, c, 0.0f ), vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ); }
inline mat44_t rotation_y( float angle ) { float s = internal_vecmath_sin( angle ); float c = internal_vecmath_cos( angle ); return mat44( vec4( c, 0.0f, -s, 0.0f ), vec4( 0.0f, 1.0f, 0.0f, 0.0f ), vec4( s, 0.0f, c, 0.0f ), vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ); }
inline mat44_t rotation_z( float angle ) { float s = internal_vecmath_sin( angle ); float c = internal_vecmath_cos( angle ); return mat44( vec4( c, s, 0.0f, 0.0f ), vec4( -s, c, 0.0f, 0.0f ), vec4( 0.0f, 0.0f, 1.0f, 0.0f ), vec4( 0.0f, 0.0f, 0.0f, 1.0f ) ); }
inline mat44_t rotation_yaw_pitch_roll( float yaw, float pitch, float roll ) { return mat44_mul_mat44( rotation_y( yaw ), mat44_mul_mat44( rotation_x( pitch ), rotation_z( roll ) ) ); }
inline mat44_t translation( float x, float y, float z ) { return mat44( vec4( 1.0f, 0.0f, 0.0f, 0.0f ), vec4( 0.0f, 1.0f, 0.0f, 0.0f ), vec4( 0.0f, 0.0f, 1.0f, 0.0f ), vec4( x, y, z, 1.0f ) ); }
inline vec4_t transform( vec4_t v, mat44_t matrix ) { return vec4_mul_mat44( v, matrix ); }


float const PI     = 3.1415926535897932384626433832795f;

int generate_rotated_album( img_t const* image, float rot_angle, float thickness_scale, uint32_t* out_img, int out_width, int out_height, int single_img )
    {
    int canvas_width = out_width;
    int canvas_height = out_height;
    uint32_t* canvas = out_img;

    float margin_x = 0.0f;
    if( !single_img ) 
        {
        for( int x = 0; x < image->width; ++x )
            {
            if( ( image->pixels[ image->width * ( image->height / 2 ) + x ] & 0xff000000 ) == 0 )
                margin_x = (float)x;
            else
                break;
            }
        margin_x /= 512.0f;
        }

    vec3_t camera_pos = vec3( 0, 0, -6.0f );
    vec3_t camera_dir = vec3( 0, 0, 0 );
    mat44_t view_matrix;
    mat44_inverse (&view_matrix, 0, mat44_mul_mat44( rotation_yaw_pitch_roll( camera_dir.y, camera_dir.x, camera_dir.z ), 
        translation( camera_pos.x, camera_pos.y, camera_pos.z ) ) );

   
    float angle = rot_angle;
    float min_angle = 15.0f;
    float max_angle = 70.0f;
    if( angle < min_angle ) angle = min_angle;
    if( angle > max_angle ) angle = max_angle;
    mat44_t rot = rotation_y( -angle * PI / 180.0f );

    float min_scr_scale = 0.35f;
    float max_scr_scale = 0.405f;
    float scr_scale = min_scr_scale + ( max_scr_scale - min_scr_scale ) * ( ( angle - min_angle ) / ( max_angle - min_angle ) );
    mat44_t projection_matrix = perspective_lh( scr_scale, scr_scale, 1.0f, 10.0f );
    mat44_t view_projection_matrix = mat44_mul_mat44(view_matrix, projection_matrix);

    mat44_t xform = mat44_mul_mat44( rot, view_projection_matrix );

    trirast_vert_t v[ 4 ];
    v[ 0 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 1.0f, 0.0f );
    v[ 1 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 5.0f / 512.0f, 0.0f );
    v[ 2 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 5.0f / 512.0f, 1.0f );
    v[ 3 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 1.0f, 1.0f );

    float scale = 1.0f;
    vec3_t p[ 4 ];
    p[ 0 ] = vec3(  scale, -scale * 1.0f, 0.0f );
    p[ 1 ] = vec3( -scale, -scale * 1.0f, 0.0f );
    p[ 2 ] = vec3( -scale,  scale * 1.0f, 0.0f );
    p[ 3 ] = vec3(  scale,  scale * 1.0f, 0.0f );

    for( int i = 0; i < 4; ++i )
        {
        vec4_t pt = transform( vec4v3f( p[ i ], 1.0f ), xform );
        v[ i ].x = pt.x / pt.w;
        v[ i ].y = pt.y / pt.w;
        v[ i ].z = pt.w;
        }

    trirast_texture( v[0], v[2], v[1], canvas, canvas_width, canvas_height, image->pixels, image->width, image->height, 0xffffffff );
    trirast_texture( v[0], v[3], v[2], canvas, canvas_width, canvas_height, image->pixels, image->width, image->height, 0xffffffff );
    int wx = (int)( ( v[ 0 ]. x ) * ( canvas_width / 2.0f ) + ( canvas_width / 2.0f ) );

    v[ 0 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 5.0f / 512.0f, 0.0f );
    v[ 1 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 0.0f, 0.0f );
    v[ 2 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 0.0f / 512.0f, 1.0f );
    v[ 3 ] = trirast_vert( 0.0f, 0.0f, 0.0f, margin_x + 5.0f / 512.0f, 1.0f );

    float min_thickness = 12.0f;
    float max_thickness = 35.0f;
    scale = min_thickness + ( max_thickness - min_thickness ) * ( 1.0f - ( angle - min_angle ) / ( max_angle - min_angle ) );
    scale *= thickness_scale;
    p[ 0 ] = vec3( -1.0f, -1.0f, 0.0f/512.0f );
    p[ 1 ] = vec3( -1.0f, -1.0f, scale/512.0f );
    p[ 2 ] = vec3( -1.0f,  1.0f, scale/512.0f );
    p[ 3 ] = vec3( -1.0f,  1.0f, 0.0f/512.0f );
    for( int i = 0; i < 4; ++i )
        {
        vec4_t pt = transform( vec4v3f( p[ i ], 1.0f ), xform );
        v[ i ].x = pt.x / pt.w;
        v[ i ].y = pt.y / pt.w;
        v[ i ].z = pt.w;
        }
    
    trirast_texture( v[0], v[2], v[1], canvas, canvas_width, canvas_height, image->pixels, image->width, image->height, 0xff404040 );
    trirast_texture( v[0], v[3], v[2], canvas, canvas_width, canvas_height, image->pixels, image->width, image->height, 0xff404040 );

    int x = (int)( ( v[ 0 ]. x ) * ( canvas_width / 2.0f ) + ( canvas_width / 2.0f ) ) - 1;
    int add[] = { 32, 96, 48 };
    for( int y = 0; y < canvas_width; ++y )
        {
        for( int i = 0; i < 3; ++i )
            {
            uint32_t c = canvas[ x + i + y * canvas_width ];
		    int r = (int)( ( c & 0x000000ff ) );
		    int g = (int)( ( c & 0x0000ff00 ) >> 8 );
		    int b = (int)( ( c & 0x00ff0000 ) >> 16 );                
		    int a = (int)( ( c & 0xff000000 ) >> 24 );                
            int av = ( add[ i ] * a ) >> 8;
            r = ( r + av );
            g = ( g + av );
            b = ( b + av );
            if( r > 255 ) r = 255;
            if( g > 255 ) g = 255;
            if( b > 255 ) b = 255;
            canvas[ x + i + y * canvas_width ] = (uint32_t)( ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r );
            }
        }

    x = (int)( ( v[ 1 ]. x ) * ( canvas_width / 2.0f ) + ( canvas_width / 2.0f ) ) - 1;
    memmove( canvas, canvas + x,  sizeof( uint32_t ) * canvas_width * ( canvas_height - 1 ) );

    return (int)( wx - x );
    }

void generate_artist_thumbnail( int images_count, img_t const* const* images, img_t* shadow, uint32_t* out_img, int out_width, int out_height )
    {
    (void) images_count, images, shadow;

    int temp_width = out_width * 2;
    int temp_height = out_height * 2;
    uint32_t* temp = (uint32_t*) malloc( temp_width * temp_height * sizeof( uint32_t ) );

    int canvas_width = out_width * 2;
    int canvas_height = out_height * 2;

    uint32_t* canvas = (uint32_t*) malloc( canvas_width * canvas_height * sizeof( uint32_t ) );
    memset( canvas, 0, canvas_width * canvas_height * sizeof( uint32_t ) );

    float thickness_scale = 1.0f;
    if( images_count == 1 ) thickness_scale = 2.0f;
    if( images_count == 2 ) thickness_scale = 1.5f;
    if( images_count == 3 ) thickness_scale = 1.25f;
    float angle = 15.0f + ( 50.0f - 15.0f ) * ( images_count / 10.0f ) ;
    memset( temp, 0, temp_width * temp_height * sizeof( uint32_t ) );
    int rotwidth = generate_rotated_album( images[ 0 ], angle, thickness_scale, temp, temp_width, temp_height, images_count > 1 ? 0 : 1 ) + 1;
    image_merge( canvas, canvas_width, canvas_height, temp, temp_width, temp_height, images_count == 1 ? ((temp_width-1) - rotwidth) / 2 : 0, 0 );
    
    for( int i = 1; i < images_count; ++i ) 
        {
        memset( temp, 0, temp_width * temp_height * sizeof( uint32_t ) );
        generate_rotated_album( images[ i ], angle, thickness_scale, temp, temp_width, temp_height, images_count > 1 ? 0 : 1 );
        int xp = ( ((temp_width-1) - rotwidth) / ( images_count - 1 == 0 ? 1 : images_count - 1) ) * i;
        image_merge( canvas, canvas_width, canvas_height, shadow->pixels, shadow->width, shadow->height, xp - shadow->width / 2 + images_count / 2, 0 );
        image_merge( canvas, canvas_width, canvas_height, temp, temp_width, temp_height, xp, 0 );
        }
        
    stbir_resize_uint8( (unsigned char*) canvas, canvas_width, canvas_height, 0, (unsigned char*) out_img, out_width, out_height, 0, 4 );
    
    free( temp );
    free( canvas );
    }
