
#ifndef TRIRAST_U32
    #define TRIRAST_U32 uint32_t
#endif

typedef struct trirast_vert_t { float x, y, z, u, v; } trirast_vert_t;

trirast_vert_t trirast_vert( float x, float y, float z, float u, float v ) 
    {
    trirast_vert_t vert;
    vert.x = x;
    vert.y = y;
    vert.z = z;
    vert.u = u;
    vert.v = v;
    return vert;
    }

__forceinline void trirast_texture( trirast_vert_t v0, trirast_vert_t v1, trirast_vert_t v2, 
    TRIRAST_U32* color_buffer, int width, int height, TRIRAST_U32 const* tex, int tex_w, int tex_h, TRIRAST_U32 modulate );


/*
----------------------
	IMPLEMENTATION
----------------------
*/
__forceinline int trirast_ceil( float x ) { return ( ( (int) x ) + 1 ); }



typedef struct gradients_t 
    {
	float one_over_z[3];				// 1/z for each vertex
	float u_over_z[3];				// u/z for each vertex
	float v_over_z[3];				// v/z for each vertex
	float one_over_z_dx, one_over_z_dy;	// d(1/z)/dX, d(1/z)/dY
	float u_over_z_dx, u_over_z_dy;		// d(u/z)/dX, d(u/z)/dY
	float v_over_z_dx, v_over_z_dy;		// d(v/z)/dX, d(v/z)/dY
    } gradients_t;


__forceinline void gradients_init( gradients_t* gradients, trirast_vert_t const *v )
    {
    float dx = ( ( v[1].x - v[2].x ) * ( v[0].y - v[2].y ) ) - ( ( v[0].x - v[2].x ) * ( v[1].y - v[2].y ) ) ;
	float one_over_dx = 1.0f / dx;
	float one_over_dy = -one_over_dx;

	for( int i = 0; i < 3; ++i )
	    {
		float one_over_z = 1.0f / v[i].z;
		gradients->one_over_z[i] = one_over_z;
		gradients->u_over_z[i] = v[i].u * one_over_z;
		gradients->v_over_z[i] = v[i].v * one_over_z;
	    }

	gradients->one_over_z_dx = one_over_dx * (
        ( gradients->one_over_z[1] - gradients->one_over_z[2] ) * ( v[0].y - v[2].y ) -
        ( gradients->one_over_z[0] - gradients->one_over_z[2] ) * ( v[1].y - v[2].y )
        );

	gradients->one_over_z_dy = one_over_dy * (
        ( gradients->one_over_z[1] - gradients->one_over_z[2]) * (v[0].x - v[2].x) -
		( gradients->one_over_z[0] - gradients->one_over_z[2]) * (v[1].x - v[2].x) 
        );

	gradients->u_over_z_dx =  one_over_dx * (
        ( gradients->u_over_z[1] - gradients->u_over_z[2] ) * ( v[0].y - v[2].y ) -
		( gradients->u_over_z[0] - gradients->u_over_z[2] ) * ( v[1].y - v[2].y )
        );
	
    gradients->u_over_z_dy = one_over_dy * (
        ( gradients->u_over_z[1] - gradients->u_over_z[2] ) * ( v[0].x - v[2].x ) -
		( gradients->u_over_z[0] - gradients->u_over_z[2] ) * ( v[1].x - v[2].x )
        );

	gradients->v_over_z_dx = one_over_dx * (
        ( gradients->v_over_z[1] - gradients->v_over_z[2] ) * ( v[0].y - v[2].y ) -
		( gradients->v_over_z[0] - gradients->v_over_z[2] ) * ( v[1].y - v[2].y )
        );

	gradients->v_over_z_dy = one_over_dy * (
        ( gradients->v_over_z[1] - gradients->v_over_z[2] ) * ( v[0].x - v[2].x ) -
		( gradients->v_over_z[0] - gradients->v_over_z[2] ) * ( v[1].x - v[2].x )
        );
    }



typedef struct edge_t 
    {
	float x; // fractional x
    float x_step; // dx/dy
	int y; // current y
    int height; // vertical count
	float one_over_z, one_over_z_step;	// 1/z and step
	float u_over_z, u_over_z_step;		// u/z and step
	float v_over_z, v_over_z_step;		// v/z and step
    } edge_t;


__forceinline void edge_init( edge_t* edge, gradients_t const* grad, trirast_vert_t const* vt, trirast_vert_t const* vb, int top )
    {
	int y_end = (int) trirast_ceil( vb->y );
	edge->y = (int) trirast_ceil( vt->y );
	edge->height = y_end - edge->y;

	float y_prestep = edge->y - vt->y;
	
	float real_height = vb->y - vt->y;
	float real_width = vb->x - vt->x;

	edge->x = ( ( real_width * y_prestep ) / real_height ) + vt->x;
	edge->x_step = real_width / real_height;
	float x_prestep = edge->x - vt->x;

	edge->one_over_z = grad->one_over_z[top] + y_prestep * grad->one_over_z_dy + x_prestep * grad->one_over_z_dx;
	edge->one_over_z_step = edge->x_step * grad->one_over_z_dx + grad->one_over_z_dy;

	edge->u_over_z = grad->u_over_z[top] + y_prestep * grad->u_over_z_dy + x_prestep * grad->u_over_z_dx;
	edge->u_over_z_step = edge->x_step * grad->u_over_z_dx + grad->u_over_z_dy;

	edge->v_over_z = grad->v_over_z[top] + y_prestep * grad->v_over_z_dy + x_prestep * grad->v_over_z_dx;
	edge->v_over_z_step = edge->x_step * grad->v_over_z_dx + grad->v_over_z_dy;
    }


__forceinline int edge_step( edge_t* edge ) 
    {
	edge->x += edge->x_step; 
    ++edge->y; 
    --edge->height;
	edge->u_over_z += edge->u_over_z_step; 
    edge->v_over_z += edge->v_over_z_step; 
    edge->one_over_z += edge->one_over_z_step;
	return edge->height;
    }



__forceinline TRIRAST_U32 interpolate_rgba( TRIRAST_U32 color1, TRIRAST_U32 color2, int alpha )
	{
	int inv_alpha = 255 - alpha;
	return
        (color1 & 0xff000000) |
		(((((color1 & 0x00ff0000)>>16)*inv_alpha)+((color2 & 0x00ff0000)>>16)*alpha)>>8)<<16 |
		(((((color1 & 0x0000ff00)>>8 )*inv_alpha)+((color2 & 0x0000ff00)>>8 )*alpha)>>8)<<8  |
		(((((color1 & 0x000000ff)    )*inv_alpha)+((color2 & 0x000000ff)    )*alpha)>>8)
        ;
	}


__forceinline TRIRAST_U32 modulate_rgba( TRIRAST_U32 color1, TRIRAST_U32 color2 )
    {
	int cr = (int)( ( color2 & 0x000000ff ) );
	int cg = (int)( ( color2 & 0x0000ff00 ) >> 8 );
	int cb = (int)( ( color2 & 0x00ff0000 ) >> 16 );                
	int r = (int)( ( color1 & 0x000000ff ) );
	int g = (int)( ( color1 & 0x0000ff00 ) >> 8 );
	int b = (int)( ( color1 & 0x00ff0000 ) >> 16 );                
	int a = (int)( ( color1 & 0xff000000 ) >> 24 );                
    r = ( r * ( 255 - cr ) ) >> 8;
    g = ( g * ( 255 - cg ) ) >> 8;
    b = ( b * ( 255 - cb ) ) >> 8;
    r = ( r - cr / 3 );
    g = ( g - cg / 3 );
    b = ( b - cb / 3 );
    if( r < 0 ) r = 0;
    if( g < 0 ) g = 0;
    if( b < 0 ) b = 0;
    return (uint32_t)( ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | r );
    }


__forceinline void draw_scanline( TRIRAST_U32* color_buffer, int color_buffer_width, gradients_t const* grad,
	edge_t const* left, edge_t const* right, TRIRAST_U32 const* tex, int tex_w, int tex_h, TRIRAST_U32 modulate )
    {
    (void) tex_h;
	int x_start = (int) trirast_ceil( left->x );
	float x_prestep = x_start - left->x;
	int x_clip = 0;
	if( x_start < 0 ) { x_clip = -x_start + 1; x_start = 0; }

	uint32_t* colbuf = color_buffer;
	colbuf += left->y * color_buffer_width + x_start;

	int width = (int) trirast_ceil( right->x ) - x_start;
	if( x_start + width >= color_buffer_width ) width -= ( x_start + width ) - color_buffer_width;

	float one_over_z = left->one_over_z + x_prestep * grad->one_over_z_dx;
	float u_over_z = left->u_over_z + x_prestep * grad->u_over_z_dx;
	float v_over_z = left->v_over_z + x_prestep * grad->v_over_z_dx;

    float x_clip_f = (float) x_clip;
	one_over_z += grad->one_over_z_dx * x_clip_f;
	u_over_z += grad->u_over_z_dx * x_clip_f;
	v_over_z += grad->v_over_z_dx * x_clip_f;

	while( (width--) > 0 )
	    {
		float z = 1.0f / one_over_z;
		float fu = u_over_z * z;
		float fv = v_over_z * z;
        
        int u = (int)( fu );
        int v = (int)( fv );        
        int u0 = ( ( u % tex_w ) + tex_w ) % tex_w;
		int v0 = ( ( v % tex_h ) + tex_h ) % tex_h;
		int u1 = ( ( ( u + 1 ) % tex_w ) + tex_w ) % tex_w;
		int v1 = ( ( ( v + 1 ) % tex_h ) + tex_h ) % tex_h;
        int ui = (int)( ( fu - (float) u ) * 256.0f );
        int vi = (int)( ( fv - (float) v ) * 256.0f );
        TRIRAST_U32 c0 = *( tex + u0 + v0 * tex_w );
        TRIRAST_U32 c1 = *( tex + u1 + v0 * tex_w );
        TRIRAST_U32 c2 = *( tex + u0 + v1 * tex_w );
        TRIRAST_U32 c3 = *( tex + u1 + v1 * tex_w );
        TRIRAST_U32 c = interpolate_rgba( interpolate_rgba( c0, c1, ui ), interpolate_rgba( c2, c3, ui ), vi );
        if( modulate != 0xffffffff )
            c = modulate_rgba( c, modulate );

        //int u = ( ( ( (int) fu ) % tex_w ) + tex_w ) % tex_w;
        //int v = ( ( ( (int) fv ) % tex_h ) + tex_h ) % tex_h;
        //TRIRAST_U32 c = *( tex + u + v * tex_w );

		*colbuf++ = c;
		one_over_z += grad->one_over_z_dx;
		u_over_z += grad->u_over_z_dx;
		v_over_z += grad->v_over_z_dx;
	    }
    }


__forceinline void draw_triangle( trirast_vert_t const *v, TRIRAST_U32* buf, int buf_w, int buf_h, 
    TRIRAST_U32 const* tex, int tex_w, int tex_h, TRIRAST_U32 modulate )
    {
	float y0 = v[0].y, y1 = v[1].y, y2 = v[2].y;

	// sort vertices in y
	int top, middle, bottom, cmp_middle, cmp_bottom;
	if( y0 < y1 ) 
        {
		if( y2 < y0 ) 
            { top = 2; middle = 0; bottom = 1; cmp_middle = 0; cmp_bottom = 1; } 
        else 
            {
			top = 0;
			if( y1 < y2 )  { middle = 1; bottom = 2; cmp_middle = 1; cmp_bottom = 2; } 
            else { middle = 2; bottom = 1; cmp_middle = 2; cmp_bottom = 1; }
		    }
	    } 
    else 
        {
		if( y2 < y1 ) 
            { top = 2; middle = 1; bottom = 0; cmp_middle = 1; cmp_bottom = 0; } 
        else 
            {
			top = 1;
			if( y0 < y2 ) { middle = 0; bottom = 2; cmp_middle = 3; cmp_bottom = 2; } 
            else { middle = 2; bottom = 0; cmp_middle = 2; cmp_bottom = 3; }
		    }
	    }

	gradients_t grad; gradients_init( &grad, v );
	edge_t top_bottom; edge_init( &top_bottom, &grad, &v[top], &v[bottom], top );
	edge_t top_middle; edge_init( &top_middle, &grad, &v[top], &v[middle], top );
	edge_t middle_bottom; edge_init( &middle_bottom, &grad, &v[middle], &v[bottom], middle );
	
    edge_t *left, *right;
	int middle_is_left;

	// the triangle is clockwise, so if bottom > middle then middle is right
	if( cmp_bottom > cmp_middle ) 
        { middle_is_left = 0; left = &top_bottom; right = &top_middle; } 
    else 
        { middle_is_left = 1; left = &top_middle; right = &top_bottom; }
    
	int height = top_middle.height;
	while( (height--) > 0 )
        {
        // TODO: vertical clipping outside of loop
		if( top_middle.y >= 0 && top_middle.y < buf_h  && top_bottom.y >= 0 && top_bottom.y < buf_h )
			draw_scanline(buf, buf_w, &grad,left,right,tex, tex_w, tex_h, modulate );
		edge_step( &top_middle ); edge_step( &top_bottom );
	    }

    if( middle_is_left ) 
        { left = &middle_bottom; right = &top_bottom; } 
    else 
        { left = &top_bottom; right = &middle_bottom; }
	
	height = middle_bottom.height;
	while( (height--) > 0 )
        {
        // TODO: vertical clipping outside of loop
		if( middle_bottom.y >= 0 && middle_bottom.y < buf_h && top_bottom.y >= 0 && top_bottom.y < buf_h )
			draw_scanline(buf, buf_w, &grad,left,right,tex, tex_w, tex_h, modulate);
		edge_step( &middle_bottom ); edge_step( &top_bottom );
	    }
    }


__forceinline void trirast_texture( trirast_vert_t v0, trirast_vert_t v1, trirast_vert_t v2, 
    TRIRAST_U32* color_buffer, int width, int height, TRIRAST_U32 const* tex, int tex_w, int tex_h, TRIRAST_U32 modulate )
	{
	trirast_vert_t verts[3];

	verts[0].u = v0.u * ( tex_w - 1 ) + 0.5f;
	verts[0].v = v0.v * ( tex_h - 1 ) + 0.5f;
                           
	verts[1].u = v1.u * ( tex_w - 1 ) + 0.5f;
	verts[1].v = v1.v * ( tex_h - 1 ) + 0.5f;
                           
	verts[2].u = v2.u * ( tex_w - 1 ) + 0.5f;
	verts[2].v = v2.v * ( tex_h - 1 ) + 0.5f;
             
	verts[0].x = v0.x * width * 0.5f + width / 2.0f;
	verts[0].y = v0.y * height * 0.5f + height / 2.0f;
	verts[0].z = v0.z;
             
	verts[1].x = v1.x * width * 0.5f + width / 2.0f;
	verts[1].y = v1.y * height * 0.5f + height / 2.0f;
	verts[1].z = v1.z;
             
	verts[2].x = v2.x * width * 0.5f + width / 2.0f;
	verts[2].y = v2.y * height * 0.5f + height / 2.0f;
	verts[2].z = v2.z;

	draw_triangle( verts, color_buffer, width, height, tex, tex_w, tex_h, modulate );
	}
