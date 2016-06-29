#version 430 core
uniform sampler2D image;
uniform int imgWidth;
uniform int imgHeight;
varying vec2 pos;

void main(void)
{
   //vec2 texCoord = vec2( pos.x / float( imgWidth ), pos.y / float( imgHeight ) );
    //   vec4 col = texture2D( image, texCoord );
        vec4 col = texelFetch( image, ivec2(pos.xy), 0 );

        gl_FragColor.r = col.r;
        gl_FragColor.g = col.g;
        gl_FragColor.b = col.b;

   return;
}
