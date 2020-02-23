#include <stdio.h>

int sq(int a)
{
	return a*a;
}

int main(void)
{
	int rgbs[3]={0xff,0x57,0x57};
	int rgb[5][3]= {
	{0xff,0x00,0x40},
	{0xa8,0x00,0x00},
	{0x70,0x00,0x57},
	{0x40,0xff,0x00},
	{0xff,0x57,0x57}};
	
	int i,r,g,b,R,G,B;

	for (i=0;i<5;i++) {
/*		printf("sr %d g %d b %d\n",rgbs[0],rgbs[1],rgbs[2]);
		printf("srg halfpoint %d\n",(rgbs[0]+rgbs[1])/2);
		printf("srb halfpoint %d\n",(rgbs[0]+rgbs[2])/2);
		printf("sgb halfpoint %d\n",(rgbs[1]+rgbs[2])/2);
		printf("r %d g %d b %d\n",rgb[i][0],rgb[i][1],rgb[i][2]);
		printf("rg halfpoint %d\n",(rgb[i][0]+rgb[i][1])/2);
		printf("rb halfpoint %d\n",(rgb[i][0]+rgb[i][2])/2);
		printf("gb halfpoint %d\n",(rgb[i][1]+rgb[i][2])/2);

		printf("rg diff %d\n",((rgbs[0]+rgbs[1])/2)-((rgb[i][0]+rgb[i][1])/2));
		printf("rb diff %d\n",((rgbs[0]+rgbs[2])/2)-((rgb[i][0]+rgb[i][2])/2));
		printf("gb diff %d\n",((rgbs[1]+rgbs[2])/2)-((rgb[i][1]+rgb[i][2])/2));

		printf("crg diff %d\n",((rgb[i][0]+rgb[i][1])/2)-((rgbs[0]+rgbs[1])/2));
		printf("crb diff %d\n",((rgb[i][0]+rgb[i][2])/2)-((rgbs[0]+rgbs[2])/2));
		printf("cgb diff %d\n",((rgb[i][1]+rgb[i][2])/2)-((rgbs[1]+rgbs[2])/2));*/

		R=(sq(rgbs[0]-rgb[i][0]));
		G=(sq(rgbs[1]-rgb[i][1]));
		B=(sq(rgbs[2]-rgb[i][2]));

		printf("R %d G %d B %d\n",R,G,B);
/*		printf("delta diff %d\n",(((rgbs[0]+rgbs[1])/2)-((rgb[i][0]+rgb[i][1])/2))*(((rgbs[0]+rgbs[2])/2)-((rgb[i][0]+rgb[i][2])/2))*(((rgbs[1]+rgbs[2])/2)-((rgb[i][1]+rgb[i][2])/2)));
		printf("cdelta diff %d\n",(((rgb[i][0]+rgb[i][1])/2)-((rgbs[0]+rgbs[1])/2))*(((rgb[i][0]+rgb[i][2])/2)-((rgbs[0]+rgbs[2])/2))*(((rgb[i][1]+rgb[i][2])/2)-((rgbs[1]+rgbs[2])/2)));*/
		printf("square diff %d\n",R+G+B);
	}

	return 0;
}
