typedef struct{
	int R;
	int G;
	int B;
}Pixel;

typedef struct{
	Pixel **pixel;
	char mnum[3];
	int line;
	int column;
	int maxcolor;
}Image;
