#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "image.cu"
#define N_THREADS 8
#define N_BLOCKS 8

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

void writeImage(char *filename, Image img, Pixel *newimg){
	FILE *fp;
	char eol = '\n';
	char filepath[60];
	char spc = ((char) 32);
	char ita[5];
	int i;

	//fp = fopen("./outImages/newsign_1.ppm", "wb+");
	sprintf(filepath, "./../outImages/out%s", filename);
	fp = fopen(filepath, "wb+");

	if(fp == NULL){	printf("File not open!\n"); return;	}

	printf("Start writing!\n");

	fwrite(img.mnum, sizeof(char), 2, fp);
	fwrite(&eol, sizeof(char), 1, fp);

	sprintf(ita, "%d", img.column);
        fwrite(ita, sizeof(char), lenHelper(img.column), fp);
	fwrite(&spc, sizeof(char), 1, fp);

	sprintf(ita, "%d", img.line);
	fwrite(ita, sizeof(char), lenHelper(img.line), fp);
	fwrite(&eol, sizeof(char), 1, fp);

	sprintf(ita, "%d", img.maxcolor);
	fwrite(ita, sizeof(char), lenHelper(img.maxcolor), fp);
	fwrite(&eol, sizeof(char), 1, fp);

	printf("writting pixels!\n");

	if(img.mnum[0] == 'P' && img.mnum[1] == '6'){
		for(i = 0; i < img.line * img.column; ++i){
				fwrite(&newimg[i].R, sizeof(unsigned char), 1, fp);
				fwrite(&newimg[i].G, sizeof(unsigned char), 1, fp);
				fwrite(&newimg[i].B, sizeof(unsigned char), 1, fp);
		}
	}else if(img.mnum[0] == 'P' && img.mnum[1] == '5'){
		for(i = 0; i < img.line; ++i){
                                fwrite(&newimg[i].G, sizeof(unsigned char), 1, fp);
                }
	}

	fclose(fp);

	printf("Finished!\n");

	return;
}

__global__ void smoothImage(int line, int column, Pixel *img, Pixel *newimg){
	int i = (blockIdx.x * blockDim.x) + threadIdx.x;
	int j = (blockIdx.y * blockDim.y) + threadIdx.y;
	int i_new = (i * column) + j;

	//Calcula o novo valor do Pixel[i][j] verificando se o pixel esta no canto para colocar como 0 os pixels fora da imagem
	if(i == 0 && j == 0){
		newimg[i_new].R = (0 + 0 + 0 +
                                   0 + img[i_new].R + img[i_new + 1].R +
                                   0 + img[i_new + column].R + img[i_new + column + 1].R)
				/ 9;
				
		newimg[i_new].G = (0 + 0 + 0 +
                                   0 + img[i_new].G + img[i_new + 1].G +
                                   0 + img[i_new + column].G + img[i_new + column + 1].G)
				/9;
				
		newimg[i_new].B = (0 + 0 + 0 +
                		0 + img[i_new].B + img[i_new + 1].B +
                		0 + img[i_new + column].B + img[i_new + column + 1].B)
				/ 9;
	}else if(i == 0 && j >= column - 1){
                newimg[i_new].R = (0 + 0 + 0 +
                                   img[i_new - 1].R + img[i_new].R + 0 +
                                   img[i_new + column - 1].R + img[i_new + column].R + 0)
                                   / 9;

                                newimg[i_new].G = (0 + 0 + 0 +
                                                        img[i_new - 1].G + img[i_new].G + 0 +
                                                        img[i_new + column - 1].G + img[i_new + column].G + 0)
                                                        / 9;

                                newimg[i_new].B = (0 + 0 + 0 +
                                                        img[i_new - 1].B + img[i_new].B + 0 +
                                                        img[i_new + column - 1].B + img[i_new + column].B + 0)
                                                        / 9;

                        }else if(i >= line - 1 && j == 0){
                                newimg[i_new].R = (0 + img[i_new - column].R + img[i_new - column + 1].R +
                                                        0 + img[i_new].R + img[i_new + 1].R +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg[i_new].G = (0 + img[i_new - column].G + img[i_new - column + 1].G +
                                                        0 + img[i_new].G + img[i_new + 1].G +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg[i_new].B = (0 + img[i_new - column].B + img[i_new - column + 1].B +
                                                        0 + img[i_new].B + img[i_new + 1].B +
                                                        img[i_new].G + img[i_new].G + 0 +
                                                        0 + 0 + 0)
                                                        / 9;

                                newimg[i_new].B = (img[i_new - column - 1].B + img[i_new - column].B + 0 +
                                                        img[i_new - 1].B + img[i_new].B + 0 +
                                                        0 + 0 + 0)
                                                        / 9;
                        }else if(i == 0){
				newimg[i_new].R = (0 + 0 + 0 +
                                                        img[i_new - 1].R + img[i_new].R + img[i_new + 1].R +
                                                        img[i_new + column - 1].R + img[i_new + column].R + img[i_new + column + 1].R)
							/ 9;

				newimg[i_new].G = (0 + 0 + 0 +
                                                        img[i_new - 1].G + img[i_new].G + img[i_new + 1].G +
                                                        img[i_new + column - 1].G + img[i_new + column].G + img[i_new + column + 1].G)
                                                        / 9;

				
				newimg[i_new].B = (0 + 0 + 0 +
                                                        img[i_new - 1].B + img[i_new].B + img[i_new + 1].B +
                                                        img[i_new + column - 1].B + img[i_new + column].B + img[i_new + column + 1].B)
                                                        / 9;

			}else if(j == 0){
				newimg[i_new].R = (0 + img[i_new - column].R + img[i_new - column + 1].R+
                                                        0 + img[i_new].R + img[i_new + 1].R +
                                                        0 + img[i_new + column].R + img[i_new + column + 1].R)
							/ 9;
				
				newimg[i_new].G = (0 + img[i_new - column].G + img[i_new - column + 1].G+
                                                        0 + img[i_new].G + img[i_new + 1].G +
                                                        0 + img[i_new + column].G + img[i_new + column + 1].G)
                                                        / 9;

					
				newimg[i_new].B = (0 + img[i_new - column].B + img[i_new - column + 1].B+
                                                        0 + img[i_new].B + img[i_new + 1].B +
                                                        0 + img[i_new + column].B + img[i_new + column + 1].B)
                                                        / 9;
			}else if(i >= line - 1){
				newimg[i_new].R = (img[i_new - column - 1].R + img[i_new - column].R + img[i_new - column + 1].R+
                                                        img[i_new - 1].R + img[i_new].R + img[i_new + 1].R +
                                                        0 + 0 + 0)
                                                        / 9;


			newimg[i_new].G = (img[i_new - column - 1].G + img[i_new - column].G + img[i_new - column + 1].G+
                                                        img[i_new - 1].G + img[i_new].G + img[i_new + 1].G +
                                                        0 + 0 + 0)
                                                        / 9;

			newimg[i_new].B = (img[i_new - column - 1].B + img[i_new - column].B + img[i_new - column + 1].B+
                                                        img[i_new - 1].B + img[i_new].B + img[i_new + 1].B +
                                                        0 + 0 + 0)
                                                        / 9;
			}else if(j >= column - 1){
				newimg[i_new].R = (img[i_new - column - 1].R + img[i_new - column].R + 0 +
                                                        img[i_new - 1].R + img[i_new].R + 0 +
                                                        img[i_new + column - 1].R + img[i_new + column].R + 0)
                                                        / 9;

				newimg[i_new].G = (img[i_new - column - 1].G + img[i_new - column].G + 0 +
                                                        img[i_new - 1].G + img[i_new].G + 0 +
                                                        img[i_new + column - 1].G + img[i_new + column].G + 0)
                                                        / 9;


				newimg[i_new].B = (img[i_new - column - 1].B + img[i_new - column].B + 0 +
                                                        img[i_new - 1].B + img[i_new].B + 0 +
                                                        img[i_new + column - 1].B + img[i_new + column].B + 0)
                                                        / 9;

			}else{
				newimg[i_new].R = (img[i_new - column - 1].R + img[i_new - column].R + img[i_new - column + 1].R+
							img[i_new - 1].R + img[i_new].R + img[i_new + 1].R +
							img[i_new + column - 1].R + img[i_new + column].R + img[i_new + column + 1].R)
							/ 9;

				newimg[i_new].G = (img[i_new - column - 1].G + img[i_new - column].G + img[i_new - column + 1].G+
                                                        img[i_new - 1].G + img[i_new].G + img[i_new + 1].G +
                                                        img[i_new + column - 1].G + img[i_new + column].R + img[i_new + column + 1].G)
                                                        / 9;

				newimg[i_new].B = (img[i_new - column - 1].B + img[i_new - column].B + img[i_new - column + 1].B+
                                                        img[i_new - 1].B + img[i_new].B + img[i_new + 1].B +
                                                        img[i_new + column - 1].B + img[i_new + column].B + img[i_new + column + 1].B)
                                                        / 9;

	}

	__syncthreads();

	return;
}

Pixel *smoothInit(Image img){
	Pixel *newimg, *retimg, *piximg, *sntimg;
	int i, j;
	clock_t start, end;
        double cpu_time_used;

	retimg = (Pixel *) malloc(sizeof(Pixel) * img.line * img.column);
	piximg = (Pixel *) malloc(sizeof(Pixel) * img.line * img.column);
	for(i = 0; i < img.line; ++i){
		for(j = 0; j < img.column; ++j){
			piximg[i * (img.column) + j] = img.pixel[i][j]; 
		}
	}
	dim3 threadsPerBlock(8, 8);
	dim3 numBlocks(img.column / threadsPerBlock.x, img.line / threadsPerBlock.y);

	cudaMalloc((void **) &newimg, sizeof(Pixel) * img.line * img.column);
	cudaMalloc((void **) &sntimg, sizeof(Pixel) * img.line * img.column);
	cudaMemcpy(sntimg, piximg, sizeof(Pixel) * img.line * img.column, cudaMemcpyHostToDevice);

	start = clock();
	smoothImage<<<numBlocks, threadsPerBlock>>>(img.line, img.column, sntimg, newimg);
	end = clock();

	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

	printf("\n-----------------------------------\nTook %f seconds to execute \n-----------------------------------\n", cpu_time_used);

	cudaMemcpy(retimg, newimg, sizeof(Pixel) * img.line * img.column, cudaMemcpyDeviceToHost);

	cudaFree(newimg);
	cudaFree(sntimg);
	free(piximg);

	return retimg;
}


int main(){
	Image img;
	Pixel *newimg;
	char filename[60];
	int i;

	scanf("%s", filename);
	printf("%s\n", filename);

	img = readImage(filename);

	newimg = smoothInit(img);

	writeImage(filename, img, newimg);

	for(i = 0; i < img.line; ++i){	free(img.pixel[i]);	}
	free(img.pixel);

        free(newimg);

	return 0;
}
