#version 450 core
//#extension GL_ARB_gpu_shader_fp64: enable

#define MAX_ITERATIONS 1000

precision highp float;
 
uniform double u_dCameraZoom;
uniform float u_Color; 
uniform float u_MaxIter;
uniform float u_CameraZoom;

uniform vec2 u_CameraPos = { 0.f, 0.f };


in vec2 pass_Position;
 
out vec4 myOutputColor;

//layout(location = 0) out vec4 FragColor;


//const float PHI=1.61803398874989484820459; //   Golden Ratio 

//float gold_noise(in vec2 xy, in float seed)
//{
//    return fract(tan(distance(xy * PHI, xy) * seed) * xy.x);
//}
//
//
//float random(vec2 st) {
//    return fract(sin(dot(st.xy, vec2(12.989, 78.233))) * 43758.543);
//}
//
//float rseed = 0.;
//vec2 random2() {
//    vec2 seed = vec2(rseed++, rseed++);
//    return vec2(random(seed + 0.342), random(seed + 0.756));
//}

vec2 DoubleMul(vec2 a, vec2 b)
{
    vec2 c;
    // c = a*b
    // (c.y+c.x) = (a.x+a.y)*(b.x+b.y);
    c.y = a.y * b.y; // smallest part
    float l = a.x * b.x; // largest part
    float r = a.x * b.y + a.y * b.x; // part in-between.
    // if we add it to the big, it might lose precision in the middle of the number
    // which would be as bad as a float, so:

    c.x = l;
    c.y += r;
    return c;
}



vec4 mapColor(float mcol) {
    return vec4(0.5 + 0.5 * cos(2.7 + mcol * 30.0 + vec3(0.0, .6, 1.0)), 1.0);
}

/////////////////////////////////////
dvec2 squareImaginary(dvec2 z) {
    dvec2 z0;
    z0.x = (z.x * z.x) - (z.y * z.y);
    z0.y = 2 * z.x * z.y;
    return z0;
}
/////////////////////////////////////
vec3 colorFunc(int iter) {
    // Color in HSV
    vec3 color = vec3(u_Color + 0.012 * iter, 1.0, 0.2 + .4 * (1.0 + sin(0.3 * iter)));

    // Convert from HSV to RGB
    // Taken from: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 m = abs(fract(color.xxx + K.xyz) * 6.0 - K.www);
    return vec3(color.z * mix(K.xxx, clamp(m - K.xxx, 0.0, 1.0), color.y));
}

/////////////////////////////////////
//--- color by Renormalizing the Mandelbrot Escape
vec3 colorFunc2(int iter, float dist2) {

    float sl = ( float(iter) - log2(log2(dist2)) + 4.0) * .0025;  
    return vec3(0.5 + 0.5 * cos(2.7 + sl * 30.0 + vec3(0.0, .6, 1.0))    );
     
}

  
 
/////////////////////////////////////
 
//void render_01()
//{
//    vec3 color = vec3(0.0, 0.0, 0.0);
// 
//    vec2 c = (2 * u_CameraZoom) * pass_Position +u_CameraPos;
//    vec2 z = c;
//
//    int iterations = 0;
 
    /*
    while (iterations < u_MaxIter)
    {
 
        vec2 dist;
        dist.x = (z.x * z.x) - (z.y * z.y) +c.x;
        dist.y = 2 * z.x * z.y + c.y;

        //fc(z) = z^2 + c
        //vec2 dist = squareImaginary(z) + c;
        float dist0 = dot(dist, dist);

       // if (length(dist) > 4.0) {
        if (dist0 > 4.0) {
            color = colorFunc2(iterations, dist0);
            break;
        }
        z = dist;

        ++iterations;
    }
    */

    //-- optimize by reordering instructions, less multiplication. 
//    vec2 dist =  vec2(z.x*z.x, z.y*z.y);
//
//    while (iterations < u_MaxIter) {
//       
//        float dist0 = dist.x + dist.y;
//
//        if (dist0 > 4.0) {
//            color = colorFunc2(iterations, dist0);
//            break;
//        }
//       
//        z.y = z.x * z.y * 2 + c.y;
//        z.x = dist.x - dist.y + c.x;
//
//        dist.x = z.x * z.x;
//        dist.y = z.y * z.y;
//
//        ++iterations;
//    }
//
//    gl_FragColor = vec4(color, 1.0);
//    return; 
//
//}
 


void render_01d()
{
    vec3 color = vec3(0.0, 0.0, 0.0);
  

    dvec2 doublePosition = dvec2(pass_Position.x, pass_Position.y);
    precise  dvec2 c =
        dvec2(
            (doublePosition.x * 2.0 * u_dCameraZoom) +   u_CameraPos.x,
             (doublePosition.y * 2.0 * u_dCameraZoom) +  u_CameraPos.y
        );
     
    dvec2 z = c;


    int iterations = 0;
    dvec2 dist = dvec2(z.x * z.x, z.y * z.y);

    while (iterations < u_MaxIter)
    {
        float dist0 = float(dist.x + dist.y);
        if (dist0 > 4) {
            color = colorFunc2(iterations, dist0);
            break;
        }
        z.y = (z.x+z.x) * z.y + c.y; 
        z.x = dist.x - dist.y + c.x;

        dist.x = z.x * z.x;
        dist.y = z.y * z.y;
        

        ++iterations;
    }
    myOutputColor = vec4(color, 1.0);
    return;

}
 
/////////////////////////////////////
//vec4 return_color()
//{
    //int iter = get_iterations();
    //if (iter == MAX_ITERATIONS)
    //{
    //    gl_FragDepth = 0.0f;
    //    return vec4(0.0f, 0.0f, 0.0f, 1.0f);
    //}
 
//    float iterations = 1.0 - float(iter) / MAX_ITERATIONS;   
 //   return vec4(iterations*1.0, iterations*.2, iterations*.5, 1.0f);
 
  //  return vec4(colorFunc(iterations), 1.0f);
//}
 
 /////////////////////////////////////
void main()
{  
      render_01d(); 
}