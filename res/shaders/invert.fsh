#ifdef GL_ES
precision mediump float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D CC_Texture0;

void main()
{
    vec4 texColor = texture2D(CC_Texture0, v_texCoord);
    gl_FragColor = vec4(vec3(1.0) - texColor.rgb, texColor.a) * v_fragmentColor;
}
