#include "seamcarving.h"
#include "c_img.h"
#include "c_img.c"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int min(double a, double b, double c){
    if (a < b){
        if (a < c){
            return a;
        } else{
            return c;
        }
    } else{
        if (b < c){
            return b;
        } else{
            return c;
        }
    }
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    // define all variables to be used
    double R_x, G_x, B_x, R_y, G_y, B_y, delta_x, delta_y;
    uint8_t e;

    // create image grad
    create_img(grad, im->height, im->width);

    // iterate through each pixel and caluculate energy
    for(int y = 0; y < im->height; y ++){
        for (int x = 0; x < im->width; x ++){

            // normal cases
            R_x = get_pixel(im, y, x+1, 0) - get_pixel(im, y, x-1, 0);
            G_x = get_pixel(im, y, x+1, 1) - get_pixel(im, y, x-1, 1);
            B_x = get_pixel(im, y, x+1, 2) - get_pixel(im, y, x-1, 2);

            R_y = get_pixel(im, y+1, x, 0) - get_pixel(im, y-1, x, 0);
            G_y = get_pixel(im, y+1, x, 1) - get_pixel(im, y-1, x, 1);
            B_y = get_pixel(im, y+1, x, 2) - get_pixel(im, y-1, x, 2);

            // wrap-around cases
            if (x == 0) {
                R_x = get_pixel(im, y, x+1, 0) - get_pixel(im, y, im->width -1, 0);
                G_x = get_pixel(im, y, x+1, 1) - get_pixel(im, y, im->width -1, 1);
                B_x = get_pixel(im, y, x+1, 2) - get_pixel(im, y, im->width -1, 2);
            } 
            if (y == 0) {
                R_y = get_pixel(im, y+1, x, 0) - get_pixel(im, im->height -1, x, 0);
                G_y = get_pixel(im, y+1, x, 1) - get_pixel(im, im->height -1, x, 1);
                B_y = get_pixel(im, y+1, x, 2) - get_pixel(im, im->height -1, x, 2);
            } 
            if (x == (im->width - 1)){
                R_x = get_pixel(im, y, 0, 0) - get_pixel(im, y, x-1, 0);
                G_x = get_pixel(im, y, 0, 1) - get_pixel(im, y, x-1, 1);
                B_x = get_pixel(im, y, 0, 2) - get_pixel(im, y, x-1, 2);
            } 
            if (y == (im->height - 1)){
                R_y = get_pixel(im, 0, x, 0) - get_pixel(im, y-1, x, 0);
                G_y = get_pixel(im, 0, x, 1) - get_pixel(im, y-1, x, 1);
                B_y = get_pixel(im, 0, x, 2) - get_pixel(im, y-1, x, 2);
            }

            delta_x = pow(R_x,2.0) + pow(G_x,2.0) + pow(B_x,2.0);
            delta_y = pow(R_y,2.0) + pow(G_y,2.0) + pow(B_y,2.0);

            // calculate pixel energy
            e = (uint8_t)((pow((delta_x + delta_y), 0.5))/10);

            // set pixel to energy
            set_pixel(*grad, y, x, e, e, e);
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    // allocate best_arr
    *best_arr = (double *)malloc(sizeof(double) * grad->height * grad->width);

    // fill in first line of best_arr with first line of grad
    for (int i = 0; i < grad->width; i ++){
        (*best_arr)[i] = get_pixel(grad, 0, i, 0);
    }

    // initialize variables to be used
    double temp1, temp2, temp3, mintemp;

    // iterate through each row of grad, adding to the least of the 3 above,
    // and adding that to best_arr
    for (int y = 1; y < grad->height; y ++){
        for (int x = 0; x < grad->width; x ++){
            temp1 = (*best_arr)[(y-1)*(grad->width) + x];
            if (x != 0){
                temp2 = (*best_arr)[(y-1)*(grad->width) + x - 1];
            } else{
                temp2 = temp1;
            }
            if (x != (grad->width - 1)){
                temp3 = (*best_arr)[(y-1)*(grad->width) + x + 1];
            } else{
                temp3 = temp1;
            }
            mintemp = min(temp1, temp2, temp3);
            (*best_arr)[y*(grad->width) + x] = get_pixel(grad, y, x, 0) + mintemp;
        }
    }
}

void recover_path(double *best, int height, int width, int **path){
    // allocates array path
    *path = (int *)malloc(sizeof(int) * height);

    // initializes variables min (used to figure out the minumum of 
    // each row) and pos (the position in the row of min)
    double min;
    int pos; 

    // goes through and find the lowest of each row, that is in the
    // 3 above it
    min = best[(height-1)*width];
    pos = 0;
    for (int x = 1; x < width; x ++){
        if (best[(height-1)*width + x] < min){
            min = best[(height-1)*width + x];
            pos = x;
        }
        (*path)[height-1] = pos;
    }


    for (int y = height - 2; y >= 0; y --){
        min = best[y*width + (*path)[y+1]];
        pos = (*path)[y+1];
        for (int x = (*path)[y+1] -1; x <= (*path)[y+1] +1; x ++){
            if (best[y*width + x] < min){
                min = best[y*width + x];
                pos = x;
            }
            (*path)[y] = pos;
        }
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    // create dest which is an rbg_img of width one less than src
    create_img(dest, src->height, (src->width -1));

    int bool;

    // write from src to dest
    for (int y = 0; y < src->height; y ++){
        bool = 0;
        for (int x = 0; x < src->width; x ++){
            //printf("%d", path[y]);
            if (path[y] == x){
                bool = 1;
            } else{
                if (bool == 0){
                    set_pixel(*dest, y, x, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), 
                        get_pixel(src, y, x, 2));
                } else{
                    set_pixel(*dest, y, x-1, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), 
                        get_pixel(src, y, x, 2));
                }
            }
        }
    }
}

void remove_seam_visualized(struct rgb_img *src, struct rgb_img **dest, int *path){
    // create dest which is an rbg_img of width one less than src
    create_img(dest, src->height, (src->width));

    // write from src to dest
    for (int y = 0; y < src->height; y ++){
        for (int x = 0; x < src->width; x ++){
            //printf("%d", path[y]);
            if (path[y] == x){
                set_pixel(*dest, y, x, 0, 0, 0);
            } else{
                set_pixel(*dest, y, x, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), 
                        get_pixel(src, y, x, 2));
            }
        }
    }
}


int main(){
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "images/HJoceanSmall.bin");
    
    for(int i = 0; i < 150; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam_visualized(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "images/testing/img%d.bin", i);
        write_img(cur_im, filename);

        // if (i == 55){
        //     for (int i = 0; i < im->height; i ++){
        //         for (int j = 0; j < im->width; j++){
        //             printf("%f, ", best[i*6 + j]);
        //         }
        //         printf("\n");
        //     }
        //     for (int i = 0; i < im->height; i ++){
        //         printf("%d, ", path[i]);
        //     }
        // }

        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);
}

// int main(){

//     struct rgb_img *test;
//     read_in_img(&test, "6x5.bin");

//     struct rgb_img *grad;

//     calc_energy(test, &grad);

//     print_grad(grad);

//     double *best_arr;

//     dynamic_seam(grad, &best_arr);

//     int *path;

//     recover_path(best_arr, grad->height, grad->width,&path);

//     for (int i = 0; i < 5; i ++){
//         for (int j = 0; j < 6; j++){
//             printf("%f, ", best_arr[i*6 + j]);
//         }
//         printf("\n");
//     }

//     for (int i = 0; i < 5; i++){
//         printf("%d ", path[i]);
//     }
// }