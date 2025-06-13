#version 330

uniform float uTime;
uniform vec2 uSize;
uniform int uIterMax;
uniform vec2 uMotionOffset;

layout(location = 0) out vec4 fragColor;

void main(void) {
    vec2 g = gl_FragCoord.xy;
	vec2 si = uSize;
	
	vec3 light_color = vec3(1.2,0.8,0.6);
	
	float t = uTime*1.0;
	vec2 uv = (g+g-si)/si.y;

	float a = atan(uv.y,uv.x);
	float l = length(uv);
	
	a = floor(a)-0.1;
	
	float c = 0.0;
	
	float s = 3.14159 / 2.0;
	//a = mod(a, s) - s * 0.5;
	
	float d = 0.1 / abs(fract(a/l) - 0.5);
	
	c += d;
	
	fragColor = vec4(c * light_color, 1.0);
}

