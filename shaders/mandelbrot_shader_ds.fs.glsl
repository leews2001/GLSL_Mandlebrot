/**
 * @brief Fragment shader for rendering Mandelbrot
 * 
 *  Emulated double precision is used in calculation.
 *  Mode 0 = double-float
 *  Mode 1 = double-double
 *
 * @param planePos, The 2D plane position attribute forwarded from the vertex shader.
 * @param u_MandelbrotMode, Flag to determine whether to render the Mandelbrot or Juliabrot set.
 * @param u_Mode, rendering precision mode [0,1,2]. 
 * @param u_ds_CameraPosX, camera x-position in double-float precision.
 * @param u_ds_CameraPosY, camera y-position in double-float precision.
 * @param u_CameraZoom, zoom level of the camera, (0., 1.]
 * @param u_MaxIter, maximum number of iterations for the Mandelbrot algorithm.
 * 
 * @return myOutputColor, pixel color in the Mandelbrot set.
 */

#version 450 core

#define MAX_ITERATIONS 1000

// using float2, we can zoom to about 2e+12, before we see blocking artifacts. 
#define float2 vec2 

// using double2, we can zoom to about 2e+30, before we see blocking artifacts.
#define double2 dvec2

#ifdef GL_ES
precision highp float;
#endif

out vec4 myOutputColor;
in vec2 planePos;

uniform int u_MandelbrotMode = 1; // = 0 if we want to render Juliabrot
uniform int u_Mode = 0;

uniform vec2 u_ds_CameraPosX  = { 0., 0.};
uniform vec2 u_ds_CameraPosY  = { 0., 0.};

// Future:
// uniform dvec2 u_dd_CameraPosX = { 0., 0.};
// uniform dvec2 u_dd_CameraPosY = { 0., 0.};

uniform float u_CameraZoom = 1.0f;
uniform float u_MaxIter = MAX_ITERATIONS;
 

/**
 * @brief color by Renormalizing the Mandelbrot Escape
 *
 */
vec3 colorFunc2(int iter, float dist2) 
{
    float sl = (float(iter) - log2(log2(dist2)) + 4.0) * .0025;
    return vec3(0.5 + 0.5 * cos(2.7 + sl * 30.0 + vec3(0.0, .6, 1.0)));

} 

/////////////////////////////////////

/**
 * @brief
 *
 */
float2 emdp_sub( const float2 ds0_, const float2 ds1_)
{
    precise float _t1 = ds0_.x - ds1_.x;
    float _e = _t1 - ds0_.x;

    precise float _t2 = ((-ds1_.x - _e) + (ds0_.x - (_t1 - _e))) + ds0_.y - ds1_.y;

    float2 _ds;
    _ds.x = _e = _t1 + _t2;
    _ds.y = _t2 - (_e - _t1);
  
    return _ds;
}

/**
 * @brief
 *
 */
double2 emdp_sub(const double2 ds0_, const double2 ds1_)
{
    precise double _t1 = ds0_.x - ds1_.x;
    double _e = _t1 - ds0_.x;

    precise double _t2 = ((-ds1_.x - _e) + (ds0_.x - (_t1 - _e))) + ds0_.y - ds1_.y;

    double2 _ds;
    _ds.x = _e = _t1 + _t2;
    _ds.y = _t2 - (_e - _t1);

    return _ds;
}

/////////////////////////////////////

/**
 * @brief
 *
 */
float2 emdp_add( const float2 ds0_, const float2 ds1_)
{
    //-- TWO-SUM ( ds0_.val, ds1_.val) [Knuth]
    precise float _x = ds0_.x + ds1_.x;

    // Note: the effective _ds1.val that is added to ds0_.val to give _x;
    //precise  
    float _ds1_val_virtual = _x - ds0_.x;

    // Note: (_x - _ds1_val_virtual) = _ds0_val_virtual, the effective _ds0.val that contributing to _x;
    // Note: _y = all the round off errors in _x;
    precise float _y = ((ds1_.x - _ds1_val_virtual) + (ds0_.x - (_x - _ds1_val_virtual))) + ds0_.y + ds1_.y;
    // Note: also add existing errors from ds0_ and ds1_
    //_y = _y + ds0_.y + ds1_.y;

    //--- FAST-TWO-SUM ( _x, _y) [Dekker], |_x| > |_y|
    precise float2 _ds;
    _ds.x = _x + _y;
    _ds.y = _y - (_ds.x - _x);

    return  _ds;
}

/**
 * @brief
 *
 */
double2 emdp_add(const double2 ds0_, const double2 ds1_)
{
    precise double _x = ds0_.x + ds1_.x;
    double _ds1_val_virtual = _x - ds0_.x;

    precise double _y = ((ds1_.x - _ds1_val_virtual) + (ds0_.x - (_x - _ds1_val_virtual))) + ds0_.y + ds1_.y;

    precise double2 _ds;
    _ds.x = _x + _y;
    _ds.y = _y - (_ds.x - _x);

    return  _ds;
}


/////////////////////////////////////

/**
 * @brief
 *
 */
float2 emdp_mul( const float2 ds0_, const float2 ds1_)
{
    //--- using SPLIT(a,s)
    precise float cona = ds0_.x * 8193.f;
    precise float conb = ds1_.x * 8193.f;
    precise float a1 = cona - (cona - ds0_.x); // hi-split
    precise float b1 = conb - (conb - ds1_.x); // hi-split
    

    float a2 = ds0_.x - a1; // lo-split
    float b2 = ds1_.x - b1; // lo-split

    //---  (c11, c21) is result of TWO-PRODUCT( ds0_.hi, ds1_.hi) [Dekker]
    float c11 = ds0_.x * ds1_.x;
    //precise 
        //float err3 = (c11 - (a1 * b1)) - (a2 * b1) - (a1 * b2);
    //precise 
        //float c21 = (a2 * b2) - err3;

    precise float c21 = (((a1 * b1 - c11) + (a1 * b2)) + (a2 * b1)) + (a2 * b2);

    //--- Compute cross hi-lo products, only hi-word is needed.
    //precise 
    float c2 = (ds0_.x * ds1_.y) + (ds0_.y * ds1_.x);

    // TWO-SUM( [c11,c21], [c2,0]), also adding low-order product.
    float t1 = c2+ c11;
    float e = t1 - c11;
    precise float t2 = ((c2 - e) + (c11 - (t1 - e))) + c21;

    // FAST-TWO-SUM (t1, t2 +  (ds0_.lo * ds1_.lo))
    t2 = t2 + (ds0_.y * ds1_.y);

    float2 _ds;
    _ds.x = t1 + t2;
    _ds.y = t2 - (_ds.x - t1);

    return  _ds;
}

/**
 * @brief
 *
 */
double2 emdp_mul(const double2 ds0_, const double2 ds1_)
{ 
    precise double cona = ds0_.x * 8193.f;
    precise double conb = ds1_.x * 8193.f;
    precise double a1 = cona - (cona - ds0_.x); // hi-split
    precise double b1 = conb - (conb - ds1_.x); // hi-split

    double a2 = ds0_.x - a1; // lo-split
    double b2 = ds1_.x - b1; // lo-split


    double c11 = ds0_.x * ds1_.x;

    precise double c21 = (((a1 * b1 - c11) + (a1 * b2)) + (a2 * b1)) + (a2 * b2);

    double c2 = (ds0_.x * ds1_.y) + (ds0_.y * ds1_.x);

    double t1 = c2 + c11;
    double e = t1 - c11;

    precise double t2 = ((c2 - e) + (c11 - (t1 - e))) + c21;

    t2 = t2 + (ds0_.y * ds1_.y);

    double2 _ds;
    _ds.x = t1 + t2;
    _ds.y = t2 - (_ds.x - t1);

    return  _ds;
}


/////////////////////////////////////

/**
 * @brief
 *
 */
float2 emdp_scale( const float2 ds0_, const float sc_)
{
    //--- using SPLIT(a,s)
    precise float cona = ds0_.x * 8193.;
    precise float conb = sc_ * 8193.;
 
    float  a1 = cona - (cona - ds0_.x); // hi-split
    float  b1 = conb - (conb - sc_);    // hi-split

    float a2 = ds0_.x - a1; // lo-split
    float b2 = sc_ - b1;    // lo-split

    //---  (c11, c21) is result of TWO-PRODUCT( ds0_.hi, ds1_.hi) [Dekker]
    float c11 = ds0_.x * sc_; 
    float err3 = (c11 - (a1 * b1)) - (a2 * b1) - (a1 * b2);
    float c21 = (a2 * b2) - err3;

    //--- Compute cross hi-lo products, only hi-word is needed.
    float c2 = (ds0_.y * sc_);

    // TWO-SUM( [c11,c21], [c2,0]), also adding low-order product.
    float t1 = c2 + c11; 
    float e = t1 - c11;
    float t2 = ((c2 - e) + (c11 - (t1 - e))) + c21;

    float2 _ds;
    _ds.x = t1 + t2;
    _ds.y = t2 - (_ds.x - t1);

    return  _ds;
}

/**
 * @brief
 *
 */
double2 emdp_scale(const double2 ds0_, const float sc_)
{
    precise double cona = ds0_.x * 8193.;
    precise double conb = sc_ * 8193.;

    double a1 = cona - (cona - ds0_.x); // hi-split
    double b1 = conb - (conb - sc_);    // hi-split

    double a2 = ds0_.x - a1; // lo-split
    double b2 = sc_ - b1;    // lo-split

    double c11 = ds0_.x * sc_;

    double err3 = (c11 - (a1 * b1)) - (a2 * b1) - (a1 * b2);

    double c21 = (a2 * b2) - err3;

    double c2 = (ds0_.y * sc_);

    double t1 = c2 + c11;
    double e = t1 - c11;
    double t2 = ((c2 - e) + (c11 - (t1 - e))) + c21;

    double2 _ds;
    _ds.x = t1 + t2;
    _ds.y = t2 - (_ds.x - t1);

    return  _ds;
}

/////////////////////////////////////

/**
 * @brief
 *
 */
void render_01_ds()
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    float2 _t1x = float2( planePos.x*2, 0.f );
    float2 _t1y = float2( planePos.y*2, 0.f );

    _t1x = emdp_scale(_t1x, u_CameraZoom);
    _t1y = emdp_scale(_t1y, u_CameraZoom);


    float2 _ds_cx = emdp_add(_t1x, u_ds_CameraPosX);
    float2 _ds_cy = emdp_add(_t1y, u_ds_CameraPosY);

    // vec2 z = c;
    // We could have set _ds_zx and _ds_zy to zero for strict adherence to the
    // numerical algorithm, but that's just one iteration step ahead. And
    // i just want to make it easier to turn this into a juliabrot renderer.
    // Note: juliabrot code is not in the render_01_dd(), yet.
    float2 _ds_zx =  _ds_cx;
    float2 _ds_zy =  _ds_cy;
    
    //-- if you want Juliabrot, just uncomment one of these sets
    //_ds_cx = float2( 0.28, 0.);  // julia set 1
    //_ds_cy = float2( 0.008, 0.); // julia set 1

    //_ds_cx = float2( -0.70176, 0.); // julia set 2
    //_ds_cy = float2(0.3842, 0.);  // julia set 2

    int iterations = 0;

    float2 _ds_dist_x = emdp_mul(_ds_zx, _ds_zx);
    float2 _ds_dist_y = emdp_mul(_ds_zy, _ds_zy);

    while (iterations < u_MaxIter)
    {
        float2 _dist0 = emdp_add(_ds_dist_x, _ds_dist_y);

        if (_dist0.x > 4.0) {
            color = colorFunc2(iterations, float(_dist0.x) );
            break;
        }

        _ds_zy = emdp_mul(_ds_zx, _ds_zy);
        _ds_zy = emdp_add(_ds_zy, _ds_zy);
        _ds_zy = emdp_add(_ds_zy, _ds_cy);

        _ds_zx = emdp_sub(_ds_dist_x, _ds_dist_y);
        _ds_zx = emdp_add(_ds_zx, _ds_cx);

        _ds_dist_x = emdp_mul(_ds_zx, _ds_zx);
        _ds_dist_y = emdp_mul(_ds_zy, _ds_zy);

        ++iterations;
    }
    myOutputColor = vec4(color, 1.0);
    return;
}

/**
 * @brief
 *
 */
void render_01_dd()
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    double2 _t1x = double2(planePos.x * 2, 0.f);
    double2 _t1y = double2(planePos.y * 2, 0.f);

    _t1x = emdp_scale(_t1x, u_CameraZoom);
    _t1y = emdp_scale(_t1y, u_CameraZoom);

    double2 _ds_cx = emdp_add(_t1x, u_ds_CameraPosX);
    double2 _ds_cy = emdp_add(_t1y, u_ds_CameraPosY);

    double2 _ds_zx = _ds_cx;
    double2 _ds_zy = _ds_cy;

    int iterations = 0;

    double2 _ds_dist_x = emdp_mul(_ds_zx, _ds_zx);
    double2 _ds_dist_y = emdp_mul(_ds_zy, _ds_zy);

    while (iterations < u_MaxIter) {

        double2 _dist0 = emdp_add(_ds_dist_x, _ds_dist_y);

        if (_dist0.x > 4.0) {
            color = colorFunc2(iterations, float(_dist0.x));
            break;
        }

        _ds_zy = emdp_mul(_ds_zx, _ds_zy);
        _ds_zy = emdp_add(_ds_zy, _ds_zy);
        _ds_zy = emdp_add(_ds_zy, _ds_cy);

        _ds_zx = emdp_sub(_ds_dist_x, _ds_dist_y);
        _ds_zx = emdp_add(_ds_zx, _ds_cx);

        _ds_dist_x = emdp_mul(_ds_zx, _ds_zx);
        _ds_dist_y = emdp_mul(_ds_zy, _ds_zy);

        ++iterations;
    }

    myOutputColor = vec4(color, 1.0);
    return;
}

/**
 * @brief
 * 
 */
void render_01_std()
{
    vec3 _color = vec3(0.0, 0.0, 0.0);

    vec2 _camPos = vec2(u_ds_CameraPosX.x, u_ds_CameraPosY.x);
    vec2 c = (2.0 * u_CameraZoom) * planePos + _camPos;
    vec2 z = c;

    int _iter = 0;
    //-- original version, non-optimize
    /* {
        while (_iter < u_MaxIter) {
            vec2 _new_z;
            // compute _new_z = |z|^{2} + c
            _new_z.x = (z.x * z.x) - (z.y * z.y) + c.x;
            _new_z.y = 2 * z.x * z.y + c.y;
            // compute |_new_z|^{2}
            float _dist = dot(_new_z, _new_z);
            if (_dist > 4.0) {
                // if |_new_z|^{2} > 2^{2}, abort and _color
                _color = _colorFunc2(_iter, _dist);
                break;
            }
            // assigning for the next iteration
            z = _new_z;
            ++_iter;
        }
    } */

    // slight optimized ver. by reordering instructions, 
    // and eliminating duplicated calculation of (z.x)^2 and (z.y)^2   

    // compute hadamard product z
    vec2 _zoz = vec2(z.x * z.x, z.y * z.y);

    while (_iter < u_MaxIter) {
        // compute |z|^{2}
        float _dist = _zoz.x + _zoz.y;

        if( _dist > 4.0) {
            // if |z|^{2} > 2^{2}, abort and color
            _color = colorFunc2( _iter, _dist);
            break;
        }

        // compute |z|^{2} + c
        // direct using z.y as placeholder for new z.y,
        // as we are not going to need it subsequently,
        // because zoz contains the values we need to calculate new z.x

        z.y = 2 * (z.x * z.y) + c.y;
        z.x = ( _zoz.x - _zoz.y) + c.x;
        _zoz = vec2( z.x*z.x, z.y*z.y);
        ++_iter;
    }

    myOutputColor = vec4( _color, 1.0);

    return;
}


/////////////////////////////////////

void main()
{
    if (u_Mode == 0) {
        // standard 32bit mode, 
        // artifacts will appear aroung zoom scale 1e+7
        render_01_std();
    }
    else if (u_Mode == 1) {
        // emulated 2x 32bit mode
        render_01_ds();
    }
    else {
        // emulated 2x 64bit mode.
        render_01_dd();
    }
 
    return;
}