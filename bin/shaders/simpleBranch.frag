#version 330

uniform vec2 uSize;

layout(location = 0) out vec4 fragColor;

void main() {
	float c = 0.0;
	if (gl_FragCoord.x > uSize.x * 0.5) {
		c = 1.0;
	}
	fragColor = vec4(c);
}