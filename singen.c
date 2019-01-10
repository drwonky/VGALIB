#include <stdio.h>
#include <math.h>

void main(void)
{
	double sinma;
	double cosma;
	int angle;

	printf("#ifndef SINCOS_H\n#define SINCOS_H\n");
	printf("int sindeg[] = {\n");
	for (angle=0;angle<361;angle++) {
		sinma = sin(-angle*M_PI/180);

		printf("%ld%s // %d %f\n", (long)(sinma*16384),angle<360?",":"",angle,sinma);
	}

	printf("};\n");
	printf("int cosdeg[] = {\n");
	for (angle=0;angle<361;angle++) {
		cosma = cos(-angle*M_PI/180);

		printf("%ld%s // %d %f\n",(long)(cosma*16384),angle<360?",":"",angle,cosma);
	}
	printf("};\n#endif\n");
}
