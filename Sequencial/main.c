#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "image.c"

int readCharNum(char stop, FILE *fp){
	char *size = (char *) calloc(sizeof(char), 1);
        int i = 0;

        do{
                fread(&size[i], sizeof(char), 1, fp);
                size = (char *) realloc(size, sizeof(char)*(++i));
        }while(size[i - 1] != stop);
        size[i - 1] = '\0';

	int num = atoi(size);
	free(size);

	return num;
}

Image readImage(char *filename){
	Image img;
        FILE *fp = NULL;
	char trash;
	char filepath[60];
	unsigned char c;
	int i, j;

        //fp = fopen("./Images/sign_1.ppm", "rb+");
	sprintf(filepath, "./../Images/%s", filename);
	printf("%s\n", filepath);

	fp = fopen(filepath, "rb+");

        if(fp == NULL){ printf("File error!\n");	}

	//Read magic number(P6 or P3)
        printf("Start\n");
	fread(&img.mnum, sizeof(char), 2, fp);
	img.mnum[2] = '\0';
	printf("%s\n", img.mnum);
	fread(&trash, sizeof(char), 1, fp);

	//Read width
	img.column = readCharNum(((char) 32), fp);
	//Read height
	img.line = readCharNum('\n', fp);
	//Read maxcolor
	img.maxcolor = readCharNum('\n', fp);

	img.pixel = (Pixel **) malloc(sizeof(Pixel) * img.line);
	
	for(i = 0; i < img.line; ++i){ img.pixel[i] = (Pixel *) malloc(sizeof(Pixel) * img.column);	}

	if(img.mnum[0] == 'P' && img.mnum[1] == '6'){
		for(i = 0; i < img.line; ++i){
			for(j = 0; j < img.column; ++j){
				fread(&c, sizeof(unsigned char), 1, fp);
				img.pixel[i][j].R = c;
				fread(&c, sizeof(unsigned char), 1, fp);
                	        img.pixel[i][j].G = c;
				fread(&c, sizeof(unsigned char), 1, fp);
	                        img.pixel[i][j].B = c;
				//printf("[%d, %d]: R: %d\tG: %d\tB: %d\n", i, j, img.pixel[i][j].R, img.pixel[i][j].G, img.pixel[i][j].B);
			}
		}
	}else if(img.mnum[0] == 'P' && img.mnum[1] == '5'){
		for(i = 0; i < img.line; ++i){
                        for(j = 0; j < img.column; ++j){
                                img.pixel[i][j].R = 0;
                                fread(&c, sizeof(unsigned char), 1, fp);
                                img.pixel[i][j].G = c;
                                img.pixel[i][j].B = 0;
                                //printf("[%d, %d]: R: %d\tG: %d\tB: %d\n", i, j, img.pixel[i][j].R, img.pixel[i][j].G, img.pixel[i][j].B);
                        }
                }
	}
	//getchar();

	fclose(fp);

	return img;
}

int lenHelper(unsigned x) {
    if(x>=1000000000) return 10;
    if(x>=100000000) return 9;
    if(x>=10000000) return 8;
    if(x>=1000000) return 7;
    if(x>=100000) return 6;
    if(x>=10000) return 5;
    if(x>=1000) return 4;
    if(x>=100) return 3;
    if(x>=10) return 2;
    return 1;
}

void writeImage(char *filename, Image newimg){
	FILE *fp;
	char eol = '\n';
	char filepath[60];
	char spc = ((char) 32);
	char ita[5];
	int i, j;

	//fp = fopen("./outImages/newsign_1.ppm", "wb+");
	sprintf(filepath, "./../outImages/out%s", filename);
	fp = fopen(filepath, "wb+");

	if(fp == NULL){	printf("File not open!\n"); return;	}

	printf("Start writing!\n");

	fwrite(newimg.mnum, sizeof(char), 2, fp);
	fwrite(&eol, sizeof(char), 1, fp);

	sprintf(ita, "%d", newimg.column);
        fwrite(ita, sizeof(char), lenHelper(newimg.column), fp);
	fwrite(&spc, sizeof(char), 1, fp);

	sprintf(ita, "%d", newimg.line);
	fwrite(ita, sizeof(char), lenHelper(newimg.line), fp);
	fwrite(&eol, sizeof(char), 1, fp);

	sprintf(ita, "%d", newimg.maxcolor);
	fwrite(ita, sizeof(char), lenHelper(newimg.maxcolor), fp);
	fwrite(&eol, sizeof(char), 1, fp);

	if(newimg.mnum[0] == 'P' && newimg.mnum[1] == '6'){
		for(i = 0; i < newimg.line; ++i){
			for(j = 0; j < newimg.column; ++j){
				fwrite(&newimg.pixel[i][j].R, sizeof(unsigned char), 1, fp);
				fwrite(&newimg.pixel[i][j].G, sizeof(unsigned char), 1, fp);
				fwrite(&newimg.pixel[i][j].B, sizeof(unsigned char), 1, fp);
			}
		}
	}else if(newimg.mnum[0] == 'P' && newimg.mnum[1] == '5'){
		for(i = 0; i < newimg.line; ++i){
                        for(j = 0; j < newimg.column; ++j){
                                fwrite(&newimg.pixel[i][j].G, sizeof(unsigned char), 1, fp);
                        }
                }
	}

	fclose(fp);

	return;
}

Image smoothImage(Image img){
	Image newimg;
	int i, j;

	newimg.mnum[0] = img.mnum[0];
	newimg.mnum[1] = img.mnum[1];
	newimg.mnum[2] = img.mnum[3];
	newimg.line = img.line;
	newimg.column = img.column;
	newimg.maxcolor = img.maxcolor;

	newimg.pixel = (Pixel **) malloc(sizeof(Pixel) * newimg.line);
	for(i = 0; i < newimg.line; ++i){	newimg.pixel[i] = (Pixel *) malloc(sizeof(Pixel) * newimg.column);	}
	
	for(i = 0; i < newimg.line; ++i){
		for(j = 0; j < newimg.column; ++j){
			//Calcula o novo valor do Pixel[i][j] verificando se o pixel esta no canto para colocar como 0 os pixels fora da imagem
			if(i == 0 && j == 0){
				printf("\nCorner Exception[%d, %d]\n", i, j);
				newimg.pixel[i][j].R = (0 + 0 + 0 +
                                                        0 + img.pixel[i][j].R + img.pixel[i][j + 1].R +
                                                        0 + img.pixel[i + 1][j].R + img.pixel[i + 1][j + 1].R)
							/ 9;
				
				newimg.pixel[i][j].G = (0 + 0 + 0 +
                                                        0 + img.pixel[i][j].G + img.pixel[i][j + 1].G +
                                                        0 + img.pixel[i + 1][j].G + img.pixel[i + 1][j + 1].G)
							/ 9;
				
				newimg.pixel[i][j].B = (0 + 0 + 0 +
                                                        0 + img.pixel[i][j].B + img.pixel[i][j + 1].B +
                                                        0 + img.pixel[i + 1][j].B + img.pixel[i + 1][j + 1].B)
							/ 9;
			}else if(i == 0 && j >= newimg.column - 1){
                                printf("\nCorner Exception[%d, %d]\n", i, j);
                                newimg.pixel[i][j].R = (0 + 0 + 0 +
                                                        img.pixel[i][j - 1].R + img.pixel[i][j].R + 0 +
                                                        img.pixel[i + 1][j - 1].R + img.pixel[i + 1][j].R + 0)
                                                        / 9;

                                newimg.pixel[i][j].G = (0 + 0 + 0 +
                                                        img.pixel[i][j - 1].G + img.pixel[i][j].G + 0 +
                                                        img.pixel[i + 1][j - 1].G + img.pixel[i + 1][j].G + 0)
                                                        / 9;

                                newimg.pixel[i][j].B = (0 + 0 + 0 +
                                                        img.pixel[i][j - 1].B + img.pixel[i][j].B + 0 +
                                                        img.pixel[i + 1][j - 1].B + img.pixel[i + 1][j].B + 0)
                                                        / 9;

                        }else if(i >= newimg.line - 1 && j == 0){
                                printf("\nCorner Exception[%d, %d]\n", i, j);
                                newimg.pixel[i][j].R = (0 + img.pixel[i - 1][j].R + img.pixel[i - 1][j + 1].R +
                                                        0 + img.pixel[i][j].R + img.pixel[i][j + 1].R +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg.pixel[i][j].G = (0 + img.pixel[i - 1][j].G + img.pixel[i - 1][j + 1].G +
                                                        0 + img.pixel[i][j].G + img.pixel[i][j + 1].G +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg.pixel[i][j].B = (0 + img.pixel[i - 1][j].B + img.pixel[i - 1][j + 1].B +
                                                        0 + img.pixel[i][j].B + img.pixel[i][j + 1].B +
                                                        0 + 0 + 0)
                                                        / 9;
			}else if(i >= newimg.line - 1 && j >= newimg.column - 1){
                                printf("\nCorner Exception[%d, %d]\n", i, j);
                                newimg.pixel[i][j].R = (img.pixel[i - 1][j - 1].R + img.pixel[i - 1][j].R + 0 +
                                                        img.pixel[i][j - 1].R + img.pixel[i][j].R + 0 +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg.pixel[i][j].G = (img.pixel[i - 1][j - 1].G + img.pixel[i - 1][j].G + 0 +
                                                        img.pixel[i][j - 1].G + img.pixel[i][j].G + 0 +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg.pixel[i][j].B = (img.pixel[i - 1][j - 1].B + img.pixel[i - 1][j].B + 0 +
                                                        img.pixel[i][j - 1].B + img.pixel[i][j].B + 0 +
                                                        0 + 0 + 0)
                                                        / 9;
                        }else if(i == 0){
				newimg.pixel[i][j].R = (0 + 0 + 0 +
                                                        img.pixel[i][j - 1].R + img.pixel[i][j].R + img.pixel[i][j + 1].R +
                                                        img.pixel[i + 1][j - 1].R + img.pixel[i + 1][j].R + img.pixel[i + 1][j + 1].R)
							/ 9;

                                newimg.pixel[i][j].G = (0 + 0 + 0 +
                                                        img.pixel[i][j - 1].G + img.pixel[i][j].G + img.pixel[i][j + 1].G +
                                                        img.pixel[i + 1][j - 1].G + img.pixel[i + 1][j].G + img.pixel[i + 1][j + 1].G)
							/ 9;

                                newimg.pixel[i][j].B = (0 + 0 + 0 +
                                                        img.pixel[i][j - 1].B + img.pixel[i][j].B + img.pixel[i][j + 1].B +
                                                        img.pixel[i + 1][j - 1].B + img.pixel[i + 1][j].B + img.pixel[i + 1][j + 1].B)
							/ 9;
			}else if(j == 0){
				newimg.pixel[i][j].R = (0 + img.pixel[i - 1][j].R + img.pixel[i - 1][j + 1].R+
                                                        0 + img.pixel[i][j].R + img.pixel[i][j + 1].R +
                                                        0 + img.pixel[i + 1][j].R + img.pixel[i + 1][j + 1].R)
							/ 9;

                                newimg.pixel[i][j].G = (0 + img.pixel[i - 1][j].G + img.pixel[i - 1][j + 1].G+
                                                        0 + img.pixel[i][j].G + img.pixel[i][j + 1].G +
                                                        0 + img.pixel[i + 1][j].G + img.pixel[i + 1][j + 1].G)
							/ 9;

                                newimg.pixel[i][j].B = (0 + img.pixel[i - 1][j].B + img.pixel[i - 1][j + 1].B+
                                                        0 + img.pixel[i][j].B + img.pixel[i][j + 1].B +
                                                        0 + img.pixel[i + 1][j].B + img.pixel[i + 1][j + 1].B)
							/ 9;
	

			}else if(i >= newimg.line - 1){
				newimg.pixel[i][j].R = (img.pixel[i - 1][j - 1].R + img.pixel[i - 1][j].R + img.pixel[i - 1][j + 1].R+
                                                        img.pixel[i][j - 1].R + img.pixel[i][j].R + img.pixel[i][j + 1].R +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg.pixel[i][j].G = (img.pixel[i - 1][j - 1].G + img.pixel[i - 1][j].G + img.pixel[i - 1][j + 1].G+
                                                        img.pixel[i][j - 1].G + img.pixel[i][j].G + img.pixel[i][j + 1].G +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg.pixel[i][j].B = (img.pixel[i - 1][j - 1].B + img.pixel[i - 1][j].B + img.pixel[i - 1][j + 1].B+
                                                        img.pixel[i][j - 1].B + img.pixel[i][j].B + img.pixel[i][j + 1].B +
                                                        0 + 0 + 0)
                                                        / 9;

			}else if(j >= newimg.column - 1){
				newimg.pixel[i][j].R = (img.pixel[i - 1][j - 1].R + img.pixel[i - 1][j].R + 0 +
                                                        img.pixel[i][j - 1].R + img.pixel[i][j].R + 0 +
                                                        img.pixel[i + 1][j - 1].R + img.pixel[i + 1][j].R + 0)
                                                        / 9;

                                newimg.pixel[i][j].G = (img.pixel[i - 1][j - 1].G + img.pixel[i - 1][j].G + 0 +
                                                        img.pixel[i][j - 1].G + img.pixel[i][j].G + 0 +
                                                        img.pixel[i + 1][j - 1].G + img.pixel[i + 1][j].G + 0)
                                                        / 9;

                                newimg.pixel[i][j].B = (img.pixel[i - 1][j - 1].B + img.pixel[i - 1][j].B + 0 +
                                                        img.pixel[i][j - 1].B + img.pixel[i][j].B + 0 +
                                                        img.pixel[i + 1][j - 1].B + img.pixel[i + 1][j].B + 0)
                                                        / 9;
			}else{
				newimg.pixel[i][j].R = (img.pixel[i - 1][j - 1].R + img.pixel[i - 1][j].R + img.pixel[i - 1][j + 1].R+
							img.pixel[i][j - 1].R + img.pixel[i][j].R + img.pixel[i][j + 1].R +
							img.pixel[i + 1][j - 1].R + img.pixel[i + 1][j].R + img.pixel[i + 1][j + 1].R)
							/ 9;

				newimg.pixel[i][j].G = (img.pixel[i - 1][j - 1].G + img.pixel[i - 1][j].G + img.pixel[i - 1][j + 1].G+
                                                        img.pixel[i][j - 1].G + img.pixel[i][j].G + img.pixel[i][j + 1].G +
                                                        img.pixel[i + 1][j - 1].G + img.pixel[i + 1][j].G + img.pixel[i + 1][j + 1].G)
							/ 9;

				newimg.pixel[i][j].B = (img.pixel[i - 1][j - 1].B + img.pixel[i - 1][j].B + img.pixel[i - 1][j + 1].B+
                                                        img.pixel[i][j - 1].B + img.pixel[i][j].B + img.pixel[i][j + 1].B +
                                                        img.pixel[i + 1][j - 1].B + img.pixel[i + 1][j].B + img.pixel[i + 1][j + 1].B)
							/ 9;
			} 
		}
	}

	return newimg;
}

int main(){
	Image img, newimg;
	char filename[60];
	int i, j;
	clock_t start, end;
	double cpu_time_used;

	scanf("%s", filename);
	printf("%s\n", filename);

	img = readImage(filename);

	start = clock();
	newimg = smoothImage(img);
	end = clock();

	writeImage(filename, newimg);

	for(i = 0; i < img.line; ++i){	free(img.pixel[i]);	}
	free(img.pixel);

	for(i = 0; i < newimg.line; ++i){  free(newimg.pixel[i]);     }
        free(newimg.pixel);

	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	printf("\n-----------------------------------\nTook %f seconds to execute \n-----------------------------------\n", cpu_time_used);

	return 0;
}
