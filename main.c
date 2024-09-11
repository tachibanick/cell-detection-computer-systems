// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_out.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// To run (win): main.exe example.bmp example_out.bmp

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cbmp.h"
#define THRESHOLD 96
#define SE_SIZE 3
#define PRECISION 12

unsigned char SE[SE_SIZE][SE_SIZE] = {
    {0, 1, 0},
    {1, 1, 1},
    {0, 1, 0}};

void rgb_to_greyscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      unsigned char *pixel = input_image[x][y];
      unsigned char avg_color = (pixel[0] + pixel[1] + pixel[2]) / 3;
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        processed_image[x][y] = avg_color;
      }
    }
  }
}

void greyscale_to_rgb(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        output_image[x][y][c] = processed_image[x][y];
      }
    }
  }
}

void apply_threshold(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      processed_image[x][y] = processed_image[x][y] > THRESHOLD ? 255 : 0;
    }
  }
}

void remove_edges(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  for (int x = 0; x < SE_SIZE / 2; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      processed_image[x][y] = 0;
      processed_image[BMP_WIDTH - 1 - x][y] = 0;
    }
  }

  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < SE_SIZE / 2; y++)
    {
      processed_image[x][y] = 0;
      processed_image[x][BMP_HEIGHT - 1 - y] = 0;
    }
  }
}

int erode_pixel(int x, int y, unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], unsigned char cloned_image[BMP_WIDTH][BMP_HEIGHT])
{
  unsigned char pixel = cloned_image[x][y];

  // Can't erode black pixel
  if (!pixel)
    return 0;

  for (int i = 0; i < SE_SIZE; i++)
  {
    for (int j = 0; j < SE_SIZE; j++)
    {
      // Do nothing if SE pixel is 0
      if (!SE[i][j])
        continue;

      // SE pixel is 1
      // Check if matching pixel in image is also 1
      if (cloned_image[x - (SE_SIZE / 2) + i][y - (SE_SIZE / 2) + j])
        continue;

      // They weren't both white. Erode original pixel
      processed_image[x][y] = 0;
      return 1;
    }
  }
  // Didn't erode anything
  return 0;
}

int erode(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  int eroded = 0;

  // Copy image before eroding processed_image
  unsigned char cloned_image[BMP_WIDTH][BMP_HEIGHT];
  memcpy(cloned_image, processed_image, sizeof(cloned_image));

  for (int x = SE_SIZE / 2; x < BMP_WIDTH - SE_SIZE / 2; x++)
  {
    for (int y = SE_SIZE / 2; y < BMP_HEIGHT - SE_SIZE / 2; y++)
    {
      eroded += erode_pixel(x, y, processed_image, cloned_image);
    }
  }
  return eroded;
}

void remove_s(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], int x0, int y0)
{
  for (int x = 0; x < PRECISION; x++)
  {
    for (int y = 0; y < PRECISION; y++)
    {
      processed_image[x0 - PRECISION / 2 + x][y0 - PRECISION / 2 + y] = 0;
    }
  }
}

int detect_around(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], int x0, int y0)
{
  for (int x = x0 - PRECISION / 2; x < x0 + PRECISION / 2; x++)
  {
    if (processed_image[x][y0 - PRECISION / 2] || processed_image[x][y0 + PRECISION / 2])
      return 1;
  }
  for (int y = y0 - PRECISION / 2; y < y0 + PRECISION / 2; y++)
  {
    if (processed_image[x0 - PRECISION / 2][y] || processed_image[x0 + PRECISION / 2][y])
      return 1;
  }
  return 0;
}

void detect(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  for (int x = (PRECISION / 2) + 1; x < (BMP_WIDTH - (PRECISION / 2) - 1); x++)
  {
    for (int y = (PRECISION / 2) + 1; y < (BMP_HEIGHT - (PRECISION / 2) - 1); y++)
    {
      if (processed_image[x][y])
      {
        if (detect_around(processed_image, x, y))
        {
          continue;
        }
        else
        {
          remove_s(processed_image, x, y);
          printf("%d %d", x, y);
        }
      }
    }
  }
}

// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT];

// Main function
int main(int argc, char **argv)
{
  // argc counts how may arguments are passed
  // argv[0] is a string with the name of the program
  // argv[1] is the first command line argument (input image)
  // argv[2] is the second command line argument (output image)

  // Checking that 2 arguments are passed
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  printf("Example program - 02132 - A1\n");

  // Load image from file
  read_bitmap(argv[1], input_image);
  rgb_to_greyscale(input_image, processed_image);

  // Do stuff
  apply_threshold(processed_image);
  remove_edges(processed_image);
  while (erode(processed_image) != 0)
  {
    detect(processed_image);
  }

  // Save image to file
  greyscale_to_rgb(processed_image, output_image);
  write_bitmap(output_image, argv[2]);

  printf("Done!\n");
  return 0;
}
