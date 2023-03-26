#include "seamcarving.h"
#include "c_img.h"
#include "c_img.c"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
    *best_arr = (double *)malloc(sizeof(double) * grad->height * grad->width);
    for (int i = 0; i < grad->width; i ++){
        (*best_arr)[i] = get_pixel(grad, 0, i, 0);
    }

    double temp1, temp2, temp3, min;

    for (int y = 1; y < grad->height; y ++){
        for (int x = 0; x < grad->width; x ++){
            temp1 = (*best_arr)[(y-1)*(grad->width) + x];
            if (x != 0){
                temp2 = (*best_arr)[(y-1)*(grad->width) + x - 1];
            }
            if (x != (grad->width - 1)){
                temp3 = (*best_arr)[(y-1)*(grad->width) + x + 1];
            }
            if (temp1 < temp2){
                min = temp1;
            } else{
                min = temp2;
            }
            if (min > temp3){
                min = temp3;
            }
            (*best_arr)[y*(grad->width) + x] = get_pixel(grad, y, x, 0) + min;
        }
    }
}

int main(){
    struct rgb_img *test;
    read_in_img(&test, "6x5.bin");

    struct rgb_img *grad;

    calc_energy(test, &grad);

    print_grad(grad);

    double *best_arr;

    dynamic_seam(grad, &best_arr);

    for (int i = 0; i < 5; i ++){
        for (int j = 0; j < 6; j++){
            printf("%f, ", best_arr[i*6 + j]);
        }
        printf("\n");
    }
}