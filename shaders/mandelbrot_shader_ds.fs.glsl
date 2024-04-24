
#version 450 core


#define _EXT128 0

#define MAX_ITERATIONS 1000

#ifdef GL_ES
precision highp float;
#endif

uniform int u_MandelbrotMode = 1; // = 0 if we want to render Juliabrot
uniform int u_Mode = 0;
uniform float u_Color;

uniform vec2 u_ds_CameraPosX = { 0.f, 0.f };
uniform vec2 u_ds_CameraPosY = { 0.f, 0.f };

uniform dvec2 u_dd_CameraPosX = { 0., 0. };
uniform dvec2 u_dd_CameraPosY = { 0., 0. };

uniform float u_CameraZoom = 1.0f;

uniform float u_MaxIter = MAX_ITERATIONS;


// if we define float2 as 'dvec2', we can zoom to about 2e+30, before we see blocking artifacts.
// if we define float2 as 'vec2', we can zoom to about 2e+12, before we see blocking artifacts. 
#if ( _EXT128 == 1)
    #define float2 dvec2
    #define float_01 double
#else
    #define float2 vec2
    #define float_01 float
#endif

#define double2 dvec2


in vec2 pass_Position;

out vec4 myOutputColor;

//--- color by Renormalizing the Mandelbrot Escape
vec3 colorFunc2(int iter, float dist2) {

    float sl = (float(iter) - log2(log2(dist2)) + 4.0) * .0025;
    return vec3(0.5 + 0.5 * cos(2.7 + sl * 30.0 + vec3(0.0, .6, 1.0)));

} 

/////////////////////////////////////
float2 ds_sub( const float2 ds0_, const float2 ds1_)
{
    precise float_01 _t1 = ds0_.x - ds1_.x;
    float_01 _e = _t1 - ds0_.x;

    precise float_01 _t2 = ((-ds1_.x - _e) + (ds0_.x - (_t1 - _e))) + ds0_.y - ds1_.y;

    float2 _ds;
    _ds.x = _e = _t1 + _t2;
    _ds.y = _t2 - (_e - _t1);
  
    return _ds;
}

double2 ds_sub(const double2 ds0_, const double2 ds1_)
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
float2 ds_add( const float2 ds0_, const float2 ds1_)
{
    //-- TWO-SUM ( ds0_.val, ds1_.val) [Knuth]
    precise float_01 _x = ds0_.x + ds1_.x;

    // Note: the effective _ds1.val that is added to ds0_.val to give _x;
    //precise  
    float_01 _ds1_val_virtual = _x - ds0_.x;

    // Note: (_x - _ds1_val_virtual) = _ds0_val_virtual, the effective _ds0.val that contributing to _x;
    // Note: _y = all the round off errors in _x;
    precise float_01 _y = ((ds1_.x - _ds1_val_virtual) + (ds0_.x - (_x - _ds1_val_virtual))) + ds0_.y + ds1_.y;
    // Note: also add existing errors from ds0_ and ds1_
    //_y = _y + ds0_.y + ds1_.y;

    //--- FAST-TWO-SUM ( _x, _y) [Dekker], |_x| > |_y|
    precise float2 _ds;
    _ds.x = _x + _y;
    _ds.y = _y - (_ds.x - _x);

    return  _ds;
}


double2 ds_add(const double2 ds0_, const double2 ds1_)
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
float2 ds_mul( const float2 ds0_, const float2 ds1_)
{
    //--- using SPLIT(a,s)
    precise float_01 cona = ds0_.x * 8193.f;
    precise float_01 conb = ds1_.x * 8193.f;
    precise float_01 a1 = cona - (cona - ds0_.x); // hi-split
    precise float_01 b1 = conb - (conb - ds1_.x); // hi-split
    

    float_01 a2 = ds0_.x - a1; // lo-split
    float_01 b2 = ds1_.x - b1; // lo-split

    //---  (c11, c21) is result of TWO-PRODUCT( ds0_.hi, ds1_.hi) [Dekker]
    float_01 c11 = ds0_.x * ds1_.x;
    //precise 
        //float err3 = (c11 - (a1 * b1)) - (a2 * b1) - (a1 * b2);
    //precise 
        //float c21 = (a2 * b2) - err3;

    precise float_01 c21 = (((a1 * b1 - c11) + (a1 * b2)) + (a2 * b1)) + (a2 * b2);

    //--- Compute cross hi-lo products, only hi-word is needed.
    //precise 
    float_01 c2 = (ds0_.x * ds1_.y) + (ds0_.y * ds1_.x);

    // TWO-SUM( [c11,c21], [c2,0]), also adding low-order product.
    float_01 t1 = c2+ c11;
    float_01 e = t1 - c11;
    precise float_01 t2 = ((c2 - e) + (c11 - (t1 - e))) + c21;

    // FAST-TWO-SUM (t1, t2 +  (ds0_.lo * ds1_.lo))
    t2 = t2 + (ds0_.y * ds1_.y);

    float2 _ds;
    _ds.x = t1 + t2;
    _ds.y = t2 - (_ds.x - t1);

    return  _ds;
}

double2 ds_mul(const double2 ds0_, const double2 ds1_)
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
float2 ds_scale( const float2 ds0_, const float sc_)
{
    //--- using SPLIT(a,s)
    precise float_01 cona = ds0_.x * 8193.;
    precise float_01 conb = sc_ * 8193.;
 
    float_01  a1 = cona - (cona - ds0_.x); // hi-split
    float_01  b1 = conb - (conb - sc_); // hi-split

    float_01 a2 = ds0_.x - a1; // lo-split
    float_01 b2 = sc_ - b1; // lo-split

    //---  (c11, c21) is result of TWO-PRODUCT( ds0_.hi, ds1_.hi) [Dekker]
    float_01 c11 = ds0_.x * sc_; 
    float_01 err3 = (c11 - (a1 * b1)) - (a2 * b1) - (a1 * b2);
    float_01 c21 = (a2 * b2) - err3;

    //--- Compute cross hi-lo products, only hi-word is needed.
    float_01 c2 = (ds0_.y * sc_);

    // TWO-SUM( [c11,c21], [c2,0]), also adding low-order product.
    float_01 t1 = c2 + c11; 
    float_01 e = t1 - c11;
    float_01 t2 = ((c2 - e) + (c11 - (t1 - e))) + c21;

    float2 _ds;
    _ds.x = t1 + t2;
    _ds.y = t2 - (_ds.x - t1);

    return  _ds;
}

double2 ds_scale(const double2 ds0_, const float sc_)
{
    precise double cona = ds0_.x * 8193.;
    precise double conb = sc_ * 8193.;

    double  a1 = cona - (cona - ds0_.x); // hi-split

    double b1 = conb - (conb - sc_); // hi-split

    double a2 = ds0_.x - a1; // lo-split
    double b2 = sc_ - b1; // lo-split

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
void render_01_ds()
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    float2 _t1x = float2( pass_Position.x*2, 0.f );
    float2 _t1y = float2( pass_Position.y*2, 0.f );

    _t1x = ds_scale(_t1x, u_CameraZoom);
    _t1y = ds_scale(_t1y, u_CameraZoom);


    float2 _ds_cx = ds_add(_t1x, u_ds_CameraPosX);
    float2 _ds_cy = ds_add(_t1y, u_ds_CameraPosY);

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

    float2 _ds_dist_x = ds_mul(_ds_zx, _ds_zx);
    float2 _ds_dist_y = ds_mul(_ds_zy, _ds_zy);

    while (iterations < u_MaxIter)
    {
        float2 _dist0 = ds_add(_ds_dist_x, _ds_dist_y);

        if (_dist0.x > 4.0) {
            color = colorFunc2(iterations, float(_dist0.x) );
            break;
        }

        _ds_zy = ds_mul(_ds_zx, _ds_zy);
        _ds_zy = ds_add(_ds_zy, _ds_zy);
        _ds_zy = ds_add(_ds_zy, _ds_cy);

        _ds_zx = ds_sub(_ds_dist_x, _ds_dist_y);
        _ds_zx = ds_add(_ds_zx, _ds_cx);

        _ds_dist_x = ds_mul(_ds_zx, _ds_zx);
        _ds_dist_y = ds_mul(_ds_zy, _ds_zy);

        ++iterations;
    }
    myOutputColor = vec4(color, 1.0);
    return;
}


void render_01_dd()
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    double2 _t1x = double2(pass_Position.x * 2, 0.f);
    double2 _t1y = double2(pass_Position.y * 2, 0.f);



    _t1x = ds_scale(_t1x, u_CameraZoom);
    _t1y = ds_scale(_t1y, u_CameraZoom);


    double2 _ds_cx = ds_add(_t1x, u_ds_CameraPosX);
    double2 _ds_cy = ds_add(_t1y, u_ds_CameraPosY);

    //vec2 z = c;
    double2 _ds_zx = _ds_cx;
    double2 _ds_zy = _ds_cy;

    int iterations = 0;

    double2 _ds_dist_x = ds_mul(_ds_zx, _ds_zx);
    double2 _ds_dist_y = ds_mul(_ds_zy, _ds_zy);

    while (iterations < u_MaxIter)
    {
        double2 _dist0 = ds_add(_ds_dist_x, _ds_dist_y);

        if (_dist0.x > 4.0) {
            color = colorFunc2(iterations, float(_dist0.x));
            break;
        }

        _ds_zy = ds_mul(_ds_zx, _ds_zy);
        _ds_zy = ds_add(_ds_zy, _ds_zy);
        _ds_zy = ds_add(_ds_zy, _ds_cy);

        _ds_zx = ds_sub(_ds_dist_x, _ds_dist_y);
        _ds_zx = ds_add(_ds_zx, _ds_cx);

        _ds_dist_x = ds_mul(_ds_zx, _ds_zx);
        _ds_dist_y = ds_mul(_ds_zy, _ds_zy);

        ++iterations;
    }
    myOutputColor = vec4(color, 1.0);
    return;
}



void main()
{
    //if ( int(gl_FragCoord.x) % 10 != 0 || int(gl_FragCoord.y) % 10 != 0) {
    //    return;
    //}
 
    if (u_Mode == 0) {
        render_01_ds();
    }
    else {
        render_01_dd();
    }
 
}