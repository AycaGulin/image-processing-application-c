#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_H 512
#define MAX_W 512

/* Image buffers used by the program */
int image[MAX_H][MAX_W];  // Main image
int work [MAX_H][MAX_W];  // Temporary buffer
int outp [MAX_H][MAX_W];  // Output buffer
/* Image size information loaded from the PGM file */
int width = 0, height = 0, maxval = 255;

/* Keeps the pixel value within the valid minimum and maximum limits */
int clamp_val(int v, int lo, int hi){
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}
/* Prevents indexes from going outside the image borders */
int clamp_idx(int i, int n){
    if (i < 0) return 0;
    if (i >= n) return n - 1;
    return i;
}
/* Copies all values from a source buffer into the main image array */
void copy_to_image_from(int src[MAX_H][MAX_W]){
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
            image[y][x] = src[y][x];
}

/* Reads characters until comment lines are skipped so the header can be parsed correctly */
void skip_comments(FILE *f){
    int c = fgetc(f);
    while (c == '#'){  // Skip full comment lines
        while (c != '\n' && c != EOF) c = fgetc(f);
        c = fgetc(f);
    }
    if (c != EOF) ungetc(c, f);  // Put back the first non comment character
}
/* Loads a P2 format image from a file and stores the pixels in the main image array */
int load_pgm(const char *path){
    FILE *fp = fopen(path, "r");
    if (fp == NULL){ printf("File could not be opened.\n"); return 0; }

    char magic[3] = {0};
    if (fscanf(fp, "%2s", magic) != 1 || magic[0] != 'P' || magic[1] != '2'){
        printf("Unsupported format. Use ASCII P2.\n"); fclose(fp); return 0;
    }

    skip_comments(fp);
    if (fscanf(fp, "%d", &width) != 1){ fclose(fp); return 0; }  // Read image width
    skip_comments(fp);
    if (fscanf(fp, "%d", &height) != 1){ fclose(fp); return 0; }  // Read image height
    skip_comments(fp);
    if (fscanf(fp, "%d", &maxval) != 1){ fclose(fp); return 0; }   // Read maximum pixel value

    if (width > MAX_W || height > MAX_H){
        printf("Image too large for buffers.\n"); fclose(fp); return 0;
    }

    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int v;
            if (fscanf(fp, "%d", &v) != 1){ fclose(fp); return 0; }
            image[y][x] = clamp_val(v, 0, maxval);  // Ensure valid pixel range
        }
    }
    fclose(fp);
    printf("Loaded: %s (%dx%d, max=%d)\n", path, width, height, maxval);
    return 1;
}
/* Saves the current image in ASCII P2 format to a file */
int save_pgm(const char *path){
    FILE *fp = fopen(path, "w");
    if (fp == NULL){ printf("Output open error.\n"); return 0; }

    fprintf(fp, "P2\n%d %d\n%d\n", width, height, maxval);  // Write PGM header
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++)
            fprintf(fp, "%d ", clamp_val(image[y][x], 0, maxval));  // Write each pixel value
        fprintf(fp, "\n");
    }
    fclose(fp);
    printf("Saved: %s\n", path);
    return 1;
}

/* Creates a larger image by repeating the nearest pixel when zooming */
void zoom_nearest(int factor){            
    int nh = height * factor, nw = width * factor;   // New height and width
    if (nh > MAX_H || nw > MAX_W){ printf("Zoom exceeds buffer.\n"); return; }
    for (int y = 0; y < nh; y++){
        int sy = y / factor;  // Source row
        for (int x = 0; x < nw; x++){
            int sx = x / factor;  // Source column
            work[y][x] = image[sy][sx];  // Copy nearest pixel
        }
    }
    height = nh; width = nw; copy_to_image_from(work);   // Update main image
    printf("Zoom %dx done.\n", factor);
}
/* Reduces the image size by averaging a block of pixels into a single value */
void shrink_average(int step){            
    int nh = height / step, nw = width / step;  // New height and width
    if (nh <= 0 || nw <= 0){ printf("Shrink too small.\n"); return; }
    for (int y = 0; y < nh; y++){
        for (int x = 0; x < nw; x++){
            int sum = 0, cnt = 0;
            for (int j = 0; j < step; j++){   // Loop over block rows
                for (int i = 0; i < step; i++){  // Loop over block columns
                    int sy = y * step + j, sx = x * step + i;
                    if (sy < height && sx < width){ sum += image[sy][sx]; cnt++; }  // Stay inside image
                }
            }
            work[y][x] = sum / (cnt ? cnt : 1);  // Store block average
        }
    }
    height = nh; width = nw; copy_to_image_from(work); // Update main image
    printf("Shrink 1/%d done.\n", step);
}
/* Smooths the image by averaging a 3 by 3 neighborhood around each pixel */
void average3x3(){
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int sum = 0;
            for (int j = -1; j <= 1; j++)
                for (int i = -1; i <= 1; i++)
                    sum += image[clamp_idx(y+j, height)][clamp_idx(x+i, width)];
            outp[y][x] = sum / 9;  // Store the average value
        }
    }
    copy_to_image_from(outp);
    printf("Average 3x3 done.\n");
}
/* Performs a mean filter which uses the same calculation as the average filter */
void mean3x3(){            
    average3x3();
    printf("Mean 3x3 done.\n");
}
/* Sorts nine values so the middle value can be used for the median filter */
void sort9(int a[9]){
    for (int i = 0; i < 8; i++){
        int m = i;
        for (int j = i + 1; j < 9; j++)
            if (a[j] < a[m]) m = j;
        int t = a[i]; a[i] = a[m]; a[m] = t;
    }
}
/* Reduces noise by choosing the middle value from a 3 by 3 neighborhood */
void median3x3(){
    int w[9];
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int k = 0;
            for (int j = -1; j <= 1; j++)
                for (int i = -1; i <= 1; i++)
                    w[k++] = image[clamp_idx(y+j, height)][clamp_idx(x+i, width)];
            sort9(w);  // Sort the nine values
            outp[y][x] = w[4];  // Middle value is the median
        }
    }
    copy_to_image_from(outp);
    printf("Median 3x3 done.\n");
}
/* Finds edges by combining horizontal and vertical Sobel responses into a magnitude image */
void sobel_edge(){
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int gx = 0, gy = 0;
            for (int j = -1; j <= 1; j++){
                for (int i = -1; i <= 1; i++){
                    int v = image[clamp_idx(y+j,height)][clamp_idx(x+i,width)];
                    /* Sobel X */
                    if (j==-1 && i==-1) gx += -1*v; if (j==-1 && i==0) gx += 0*v;  if (j==-1 && i==1) gx += 1*v;
                    if (j== 0 && i==-1) gx += -2*v; if (j== 0 && i==0) gx += 0*v;  if (j== 0 && i==1) gx += 2*v;
                    if (j== 1 && i==-1) gx += -1*v; if (j== 1 && i==0) gx += 0*v;  if (j== 1 && i==1) gx += 1*v;
                    /* Sobel Y */
                    if (j==-1 && i==-1) gy += -1*v; if (j==-1 && i==0) gy += -2*v; if (j==-1 && i==1) gy += -1*v;
                    if (j== 0 && i==-1) gy +=  0*v; if (j== 0 && i==0) gy +=  0*v; if (j== 0 && i==1) gy +=  0*v;
                    if (j== 1 && i==-1) gy +=  1*v; if (j== 1 && i==0) gy +=  2*v; if (j== 1 && i==1) gy +=  1*v;
                }
            }
            int mag = (int)(sqrt((double)gx*gx + (double)gy*gy) + 0.5);  // gradient strength at this pixel
            outp[y][x] = clamp_val(mag, 0, maxval);
        }
    }
    copy_to_image_from(outp);  // write the edge magnitude back to the main image
    printf("Sobel done.\n");
}
/* Finds edges by combining horizontal and vertical Prewitt responses into a magnitude image */
void prewitt_edge(){
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int gx = 0, gy = 0;
            for (int j = -1; j <= 1; j++){
                for (int i = -1; i <= 1; i++){
                    int v = image[clamp_idx(y+j,height)][clamp_idx(x+i,width)];
                    gx += (i==-1 ? -1*v : (i==1 ? 1*v : 0));  // horizontal response
                    gy += (j==-1 ? -1*v : (j==1 ? 1*v : 0));  // vertical response
                }
            }
            int mag = (int)(sqrt((double)gx*gx + (double)gy*gy) + 0.5);  // gradient strength at this pixel
            outp[y][x] = clamp_val(mag, 0, maxval);
        }
    }
    copy_to_image_from(outp);  // write the edge magnitude back to the main image
    printf("Prewitt done.\n");
}
/* Blurs the image with a five by five integer Gaussian to reduce noise */
void gaussian5x5_inplace(){
    int K[5][5] = {{2,4,5,4,2},{4,9,12,9,4},{5,12,15,12,5},{4,9,12,9,4},{2,4,5,4,2}};
    int denom = 0; for (int r=0;r<5;r++) for (int c=0;c<5;c++) denom += K[r][c];  // kernel sum
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int sum = 0;
            for (int j = -2; j <= 2; j++)
                for (int i = -2; i <= 2; i++)
                    sum += image[clamp_idx(y+j,height)][clamp_idx(x+i,width)] * K[j+2][i+2];
            work[y][x] = sum / denom;  // blurred pixel
        }
    }
    copy_to_image_from(work);   // write back blur
}
/* Canny edge detector in four steps
   blur to reduce noise
   sobel magnitude and direction buckets
   non maximum suppression along the gradient
   double threshold with neighbor promotion for weak edges */
void canny_edge_simple(){
    gaussian5x5_inplace();  // step one blur
    int maxMag = 0;
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            int gx = 0, gy = 0;
            for (int j = -1; j <= 1; j++){
                for (int i = -1; i <= 1; i++){
                    int v = image[clamp_idx(y+j,height)][clamp_idx(x+i,width)];
                     // sobel x
                    if (j==-1 && i==-1) gx += -1*v; if (j==-1 && i==0) gx += 0*v;  if (j==-1 && i==1) gx += 1*v;
                    if (j== 0 && i==-1) gx += -2*v; if (j== 0 && i==0) gx += 0*v;  if (j== 0 && i==1) gx += 2*v;
                    if (j== 1 && i==-1) gx += -1*v; if (j== 1 && i==0) gx += 0*v;  if (j== 1 && i==1) gx += 1*v;
                    // sobel y
                    if (j==-1 && i==-1) gy += -1*v; if (j==-1 && i==0) gy += -2*v; if (j==-1 && i==1) gy += -1*v;
                    if (j== 0 && i==-1) gy +=  0*v; if (j== 0 && i==0) gy +=  0*v; if (j== 0 && i==1) gy +=  0*v;
                    if (j== 1 && i==-1) gy +=  1*v; if (j== 1 && i==0) gy +=  2*v; if (j== 1 && i==1) gy +=  1*v;
                }
            }
            int mag = (int)(sqrt((double)gx*gx + (double)gy*gy) + 0.5);
            work[y][x] = mag;  // store magnitude
            if (mag > maxMag) maxMag = mag;

            double ang = atan2((double)gy,(double)gx) * 180.0 / 3.1415926535;
            if (ang < 0) ang += 180.0;   // map into half circle
            if (ang < 22.5 || ang >= 157.5) outp[y][x] = 0;
            else if (ang < 67.5)           outp[y][x] = 45;
            else if (ang < 112.5)          outp[y][x] = 90;
            else                            outp[y][x] = 135;  // direction bucket
        }
    }
    // non maximum suppression writes thinned edges into image
    for (int y = 1; y < height - 1; y++){
        for (int x = 1; x < width - 1; x++){
            int m = work[y][x], m1 = 0, m2 = 0, d = outp[y][x];
            if (d == 0){    m1 = work[y][x-1];   m2 = work[y][x+1]; }
            else if (d==45){m1 = work[y-1][x+1]; m2 = work[y+1][x-1];}
            else if (d==90){m1 = work[y-1][x];   m2 = work[y+1][x];  }
            else {          m1 = work[y-1][x-1]; m2 = work[y+1][x+1];}
            image[y][x] = (m >= m1 && m >= m2) ? m : 0;
        }
    }
    // double threshold and single pass neighbor promotion for weak edges
    int HI = (int)(maxMag * 0.20), LO = (int)(maxMag * 0.10);
    for (int y = 1; y < height - 1; y++){
        for (int x = 1; x < width - 1; x++){
            int v = image[y][x];
            if (v >= HI) image[y][x] = 255;
            else if (v >= LO){
                int strong = 0;
                for (int dy = -1; dy <= 1; dy++)
                    for (int dx = -1; dx <= 1; dx++)
                        if (image[y+dy][x+dx] >= HI) strong = 1;
                image[y][x] = strong ? 255 : 0;
            } else image[y][x] = 0;
        }
    }
    // clear a one pixel frame for a clean border
    for (int x = 0; x < width; x++){ image[0][x] = 0; image[height-1][x] = 0; }
    for (int y = 0; y < height; y++){ image[y][0] = 0; image[y][width-1] = 0; }
    printf("Canny (simple) done.\n");
}

/* Computes the local binary pattern for each pixel using the greater or equal rule */
void lbp_compute(){
    for (int y = 1; y < height - 1; y++){
        for (int x = 1; x < width - 1; x++){
            int c = image[y][x], code = 0;
            if (image[y-1][x-1] >= c) code += 128;  // bit 7 top left
            if (image[y-1][x  ] >= c) code +=  64;  // bit 6 top
            if (image[y-1][x+1] >= c) code +=  32;  // bit 5 top right
            if (image[y  ][x+1] >= c) code +=  16;  // bit 4 right
            if (image[y+1][x+1] >= c) code +=   8;  // bit 3 bottom right
            if (image[y+1][x  ] >= c) code +=   4;  // bit 2 bottom
            if (image[y+1][x-1] >= c) code +=   2;  // bit 1 bottom left
            if (image[y  ][x-1] >= c) code +=   1;  // bit 0 left
            outp[y][x] = code;                      // final eight bit code
        }
    }
    maxval = 255; copy_to_image_from(outp);  // write back result as a grayscale image
    printf("LBP done.\n");
}

/* Prints the menu shown to the user and asks for a choice */
void print_menu(){
    printf("\n--- MENU ---\n");
    printf("1 - Load Image(PGM)\n");
    printf("2 - Zoom/Shrink Image\n");
    printf("3 - Apply Filter\n");
    printf("4 - Edge Detection\n");
    printf("5 - Compute LBP\n");
    printf("6 - Save Image\n");
    printf("0 - Exit\n");
    printf("Choice: ");
}
/* Runs the main loop that reads commands and calls the selected operation */
int main(void){
    int run = 1;
    while (run){
        int c; print_menu();
        if (scanf("%d", &c) != 1){ printf("Input error.\n"); return 0; }  // stop if input cannot be read

        if (c == 1){
            char p[256]; printf("Enter PGM path: "); scanf("%255s", p);  // read file path from user
            if (!load_pgm(p)) printf("Load failed.\n");
        } else if (c == 2){
            if (width == 0 || height == 0){ printf("Load first.\n"); continue; }  // must have an image before resizing
            printf("1=Zoom2x  2=Zoom3x  3=Shrink1/2  4=Shrink1/4: ");
            int m; scanf("%d", &m);
            if (m == 1)      zoom_nearest(2);  // enlarge by two using nearest pixel
            else if (m == 2) zoom_nearest(3);  // enlarge by three using nearest pixel
            else if (m == 3) shrink_average(2); // reduce by two using block average
            else if (m == 4) shrink_average(4); // reduce by four using block average
            else printf("Unknown.\n");
        } else if (c == 3){
            if (width == 0 || height == 0){ printf("Load first.\n"); continue; }  // need an image before filtering
            printf("Filter: 1=Average 3x3, 2=Mean 3x3, 3=Median 3x3: ");
            int f; scanf("%d", &f);
             if      (f == 1) average3x3();   // smooth using average of nine neighbors
            else if (f == 2) mean3x3();      // separate option for mean as required
            else if (f == 3) median3x3();    // denoise using the median value
            else printf("Unknown.\n");
        } else if (c == 4){
            if (width == 0 || height == 0){ printf("Load first.\n"); continue; }  // need an image before edge detection
            printf("Edge: 1=Sobel  2=Prewitt  3=Canny: ");
            int e; scanf("%d", &e);
            if      (e == 1) sobel_edge();       // gradient based edges with Sobel
            else if (e == 2) prewitt_edge();     // gradient based edges with Prewitt
            else if (e == 3) canny_edge_simple(); // multi step Canny pipeline
            else printf("Unknown.\n");
        } else if (c == 5){
            if (width == 0 || height == 0){ printf("Load first.\n"); continue; }  // need an image before LBP
            lbp_compute();  // compute local binary pattern codes
        } else if (c == 6){
            if (width == 0 || height == 0){ printf("Load first.\n"); continue; }  // need an image before saving
            char o[256]; printf("Output filename: "); scanf("%255s", o);  // read output file name
            save_pgm(o);  // write current image to disk
        } else if (c == 0){
            run = 0;  // exit the program loop
        } else {
            printf("Unknown.\n");  // notify about invalid menu choice
        }
    }
    printf("Exiting.\n");
    return 0;
}
