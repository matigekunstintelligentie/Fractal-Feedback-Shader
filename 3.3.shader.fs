#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 ourColor;

uniform sampler2D webcam;
uniform sampler2D feedback;

uniform int julia_mode;
uniform float zoom;
uniform float x_pan;
uniform float y_pan;
uniform float x_offset;
uniform float y_offset;



uniform vec2 julia;

vec3 rgb2hsv(vec3 rgb){
    float cmax = max(rgb.r, max(rgb.g, rgb.b)); // maximum of r, g, b
    float cmin = min(rgb.r, min(rgb.g, rgb.b)); // minimum of r, g, b
    float diff = cmax - cmin; // diff of cmax and cmin.
    float h = -1, s = -1;

    if(cmax == cmin){
        h = 0;
    }

    else if (cmax == rgb.r){
        h = (60 * ((rgb.g - rgb.b) / diff) + 360);
        h = h - (360 * floor(h/360));
    }

    else if (cmax == rgb.g){
        h = (60 * ((rgb.b - rgb.r) / diff) + 120);
        h = h - (360 * floor(h/360));
    }

    else if (cmax == rgb.b){
        h = (60 * ((rgb.r - rgb.g) / diff) + 240);
        h = h - (360 * floor(h/360));
    }

    if (cmax == 0){
        s = 0;
    }
    else{
        s = (diff / cmax) * 100;
    }

    float v = cmax * 100;

    return vec3(h/255.,s/255.,v/255.);
}

vec3 rgb(float hue){
    int h = int(hue * 256 * 6);
    float x = (h % 0x100)/255.0;

    float r = 1.0, g = 1.0, b = 1.0;
    switch (h / 256){
    case 0: r = 1.0; g = x;       break;
    case 1: g = 1.0; r = 1.0 - x; break;
    case 2: g = 1.0; b = x;       break;
    case 3: b = 1.0; g = 1.0 - x; break;
    case 4: b = 1.0; r = x;       break;
    case 5: r = 1.0; b = 1.0 - x; break;
    }

    vec3 colors = vec3(r, g, b);

    return colors;
}

void main()
{

    //vec4 outputColor = texture(feedback, TexCoord);
    vec4 outputColor = mix(texture(feedback, TexCoord), texture(webcam, TexCoord), 0.5);

    float x = 0.0;
    float y = 0.0;
    float xx = 0.0;
    float yy = 0.0;
    float xy = 0.0;

    float julia_c1 = julia.x;
    float julia_c2 = julia.y;

    float c1 = (1.0/zoom)*(gl_FragCoord.x + x_offset + x_pan);
    float c2 = (1.0/zoom)*(gl_FragCoord.y + y_offset + y_pan);

    int i = 0;
    int flag = 0;
    int max_iter = 1000;

    bool normalised_iteration_count = true;

    bool image_mode = true;


    if(julia_mode>0){
        x = c1;
        y = c2;
        c1 = julia_c1;
        c2 = julia_c2;
    }

    while(i<max_iter){
        xx = x*x;
        yy = y*y;
        if((xx + yy) > 4.0){
            if(flag<1){
                if(normalised_iteration_count){
                    float logzn = log(xx+yy);
                    float nu = log(logzn/log(4.0))/log(2.0);
                    float iter = i + 1 - nu;

                    //outputColor.rgb = vec3(ourColor.r*iter/max_iter,ourColor.g*iter/max_iter,ourColor.b*iter/max_iter);
                    //outputColor.rgb = vec3(iter/max_iter,iter/max_iter,iter/max_iter);
                    outputColor.rgb = rgb2hsv(rgb(iter/max_iter));
                }
                else{
                    //outputColor.rgb = vec3(ourColor.r*i/max_iter, ourColor.g*i/max_iter, ourColor.b*i/max_iter);
                    //outputColor.rgb = vec3(i/max_iter, i/max_iter, i/max_iter);
                    outputColor.rgb = rgb2hsv(rgb(i/max_iter));
                }
            }
            break;
        }
        xy = x*y;

        x = xx - yy + c1;
        y = xy+xy + c2;

        if(image_mode && x > 0.f  && y > 0.0){
        //if(image_mode && x*y > 1.0f){
            //vec4 tmp = texture(feedback, vec2(x,y));
            vec4 tmp = mix(texture(feedback, vec2(x,y)), texture(webcam, vec2(x,y)), 0.5);
            if(tmp[0] > 0.0){
                outputColor.rgba = tmp;
                flag = 1;
            }
        }

        i++;
    }

    //FragColor = texture(texture1, TexCoord);
    FragColor = outputColor;
}
