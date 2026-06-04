# Image Processing Application in C

## Project Description

This project is a menu-based image processing application developed in the C programming language. The application works with grayscale PGM images and allows the user to load an image, apply different image processing operations, and save the processed result.

The project focuses on basic image processing techniques such as image resizing, smoothing filters, edge detection, and texture analysis. It was designed as a practical C programming project to understand how pixel values are stored, accessed, modified, and processed through matrices.

## Technologies Used

- C Programming Language
- Standard C Libraries
- PGM Image Format
- GCC Compiler

## Features

- Load ASCII PGM images in P2 format
- Save processed images in PGM format
- Zoom images using nearest-neighbor scaling
- Shrink images using block averaging
- Apply average filter
- Apply mean filter
- Apply median filter
- Detect edges using Sobel edge detection
- Detect edges using Prewitt edge detection
- Apply a simplified Canny edge detector
- Compute Local Binary Pattern values for texture analysis
- Use a text-based interactive menu

## Project Structure

```text
image-processing-application-c
├── README.md
└── src
    └── image_processing_application.c
```

## Implemented Image Processing Operations

### 1. PGM Image Loading

The program loads grayscale images in ASCII PGM P2 format. It reads the image width, height, maximum pixel value, and pixel intensity values into a two-dimensional matrix.

### 2. Image Resizing

The application supports both zooming and shrinking operations.

Zooming is implemented using nearest-neighbor interpolation. For example, when the image is zoomed by a factor of 2, each original pixel is expanded into a larger block.

Shrinking is implemented by averaging blocks of pixels. For example, when the image is reduced by a factor of 2, each 2 by 2 pixel block is converted into one representative pixel value.

### 3. Image Filtering

The program includes three filtering operations:

- Average filter
- Mean filter
- Median filter

Average and mean filters are used for smoothing the image by replacing each pixel with the average value of its neighboring pixels. The median filter replaces each pixel with the median value in its neighborhood and is useful for reducing noise.

### 4. Edge Detection

The application includes three edge detection methods:

- Sobel edge detection
- Prewitt edge detection
- Simplified Canny edge detection

Sobel and Prewitt filters detect sharp changes in pixel intensity by calculating horizontal and vertical gradients. The simplified Canny edge detector applies Gaussian smoothing, gradient calculation, non-maximum suppression, and thresholding.

### 5. Local Binary Pattern

The program computes Local Binary Pattern values for texture analysis. For each pixel, the surrounding eight neighbors are compared with the center pixel. The result is converted into an 8-bit binary code and stored as a grayscale value.

### 6. Saving the Processed Image

After applying image processing operations, the user can save the final processed image as a PGM file.

## How to Compile and Run

### Using GCC

Open a terminal in the project folder and compile the program with:

```bash
gcc src/image_processing_application.c -o image_processing_application -lm
```

Then run the program with:

```bash
./image_processing_application
```

On Windows, the program can be run as:

```bash
image_processing_application.exe
```

## Program Menu

The application uses a text-based menu:

```text
1 - Load Image(PGM)
2 - Zoom/Shrink Image
3 - Apply Filter
4 - Edge Detection
5 - Compute LBP
6 - Save Image
0 - Exit
```

The user selects an operation from the menu, applies it to the loaded image, and can continue processing until choosing the exit option.

## Example Workflow

1. Run the program.
2. Select `1` to load a PGM image.
3. Select `2` to zoom or shrink the image.
4. Select `3` to apply a smoothing filter.
5. Select `4` to apply edge detection.
6. Select `5` to compute Local Binary Pattern values.
7. Select `6` to save the processed image.
8. Select `0` to exit the program.

## Purpose of the Project

The purpose of this project is to practice C programming through a practical image processing application. The project demonstrates file handling, two-dimensional arrays, functions, matrix-based pixel operations, filtering algorithms, edge detection methods, and texture analysis.

## Project Outcome

This project demonstrates the ability to implement image processing algorithms from scratch in C. It also shows how a text-based application can be designed to process grayscale images through loading, modifying, analyzing, and saving image data.
