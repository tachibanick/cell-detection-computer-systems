// To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
// To run (linux/mac): ./main.out example.bmp example_out.bmp

// To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
// To run (win): main.exe example.bmp example_out.bmp

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cbmp.h"
#define THRESHOLD_OFFSET (-20)
#define SE_SIZE 5
#define SE_HALF_SIZE ((SE_SIZE) / 2)
#define PRECISION 11
#define PRECISION_HALF ((PRECISION) / 2)
#define FILTER_ZONE_SIZE ((PRECISION) + 2)
#define FILTER_ZONE_HALF ((FILTER_ZONE_SIZE) / 2)
#define FILL_CIRCLE_SIZE 20
#define FILL_CIRCLE_HALF ((FILL_CIRCLE_SIZE) / 2)
// prints step by step
#define print_cell_detection 0
// ratio between how many cells inside detection to in the filtration layer
#define RATIO_INSIDE_OUT 10
// how many cells must be at least detected inside the detection layer
#define MIN_INSIDE_CELLS 1

int cell_positions[400][2] = {{0}};

// unsigned char SE[SE_SIZE][SE_SIZE] = {
//     {0, 1, 0},
//     {1, 1, 1},
//     {0, 1, 0}};

unsigned char SE[SE_SIZE][SE_SIZE] = {
    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0},

    {1, 1, 1, 1, 1},

    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0},
};
// Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT];

unsigned char detection_zone[PRECISION][PRECISION] = {
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0}};

unsigned char filter_zone[FILTER_ZONE_SIZE][FILTER_ZONE_SIZE] = {
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0}};

// 20x20
unsigned char filled_circle[FILL_CIRCLE_SIZE][FILL_CIRCLE_SIZE] = {
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}};

// // 15x15
// unsigned char filled_circle[FILL_CIRCLE_SIZE][FILL_CIRCLE_SIZE] = {
//     {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
//     {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
//     {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
//     {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
//     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//     {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//     {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
//     {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
//     {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0},
//     {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}};

unsigned char heart[12][13] = {
    {0, 0, 2, 2, 2, 0, 0, 0, 2, 2, 2, 0, 0}, // Top part of the heart with edges as 2
    {0, 2, 1, 1, 1, 2, 0, 2, 1, 1, 4, 2, 0}, // Edges as 2
    {2, 1, 3, 3, 1, 1, 2, 1, 1, 1, 1, 4, 2}, // Top part with edges 2
    {2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 4, 2}, // More edge rows
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 2},
    {2, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 4, 2},
    {0, 2, 1, 1, 1, 1, 1, 1, 1, 1, 4, 2, 0},
    {0, 0, 2, 1, 1, 1, 1, 1, 1, 4, 2, 0, 0},
    {0, 0, 0, 2, 1, 1, 1, 1, 4, 2, 0, 0, 0},
    {0, 0, 0, 0, 2, 1, 1, 4, 2, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 2, 4, 2, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0},
};

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

void apply_threshold(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], unsigned char threshold)
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      processed_image[x][y] = processed_image[x][y] > threshold ? 255 : 0;
    }
  }
}

void get_histogram(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], uint32_t histogram[256])
{
  // Initialize histogram
  for (int i = 0; i < 256; i++)
  {
    histogram[i] = 0;
  }

  // Calculate histogram
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      histogram[processed_image[x][y]]++;
    }
  }
}

uint32_t get_pixel_sum(uint32_t histogram[256])
{
  uint32_t sum_total = 0;
  for (int t = 0; t < 256; t++)
  {
    sum_total += t * histogram[t];
  }
  return sum_total;
}

double get_otsu_threshold_variance(uint32_t histogram[256], unsigned char threshold, uint32_t *total_pixels, uint32_t *sum_total, uint32_t *sum_background, uint32_t *weight_background, uint32_t *weight_foreground)
{
  *weight_background += histogram[threshold];
  if (*weight_background == 0)
    return 0;

  *weight_foreground = *total_pixels - *weight_background;
  if (*weight_foreground == 0)
    return 0;

  *sum_background += threshold * histogram[threshold];
  double mean_background = (double)*sum_background / *weight_background;
  double mean_foreground = (double)(*sum_total - *sum_background) / *weight_foreground;

  // Calculate inter-class variance
  return (double)*weight_background * *weight_foreground *
         (mean_background - mean_foreground) * (mean_background - mean_foreground);
}

unsigned char get_otsu_threshold(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  uint32_t histogram[256];
  get_histogram(processed_image, histogram);

  uint32_t total_pixels = BMP_WIDTH * BMP_HEIGHT;
  uint32_t sum_total = get_pixel_sum(histogram); // Sum of pixel values in the image

  uint32_t sum_background = 0; // Sum of pixel values in the background

  uint32_t weight_background = 0; // Number of pixels in the background
  uint32_t weight_foreground = 0; // Number of pixels in the foreground

  double var_max = 0;
  int threshold = 0;

  // Try all thresholds to find the one that best splits the foreground and background
  for (int t = 0; t < 256; t++)
  {
    double var_between = get_otsu_threshold_variance(histogram, t, &total_pixels, &sum_total, &sum_background, &weight_background, &weight_foreground);

    if (weight_background != 0 && weight_foreground == 0)
      break;

    // Update if new maximum variance is found
    if (var_between > var_max)
    {
      var_max = var_between;
      threshold = t;
    }
  }

  return threshold;
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
      int x_check = x - SE_HALF_SIZE + i;
      int y_check = y - SE_HALF_SIZE + j;

      // Check if pixel is outside image and erode it
      if (x_check < 0 || x_check >= BMP_WIDTH || y_check < 0 || y_check >= BMP_HEIGHT)
      {
        processed_image[x][y] = 0;
        return 1;
      }

      // Check if matching pixel in image is also 1
      if (cloned_image[x_check][y_check])
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

  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      eroded += erode_pixel(x, y, processed_image, cloned_image);
    }
  }
  return eroded;
}

int dilate_pixel(int x, int y, unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], unsigned char cloned_image[BMP_WIDTH][BMP_HEIGHT])
{
  unsigned char pixel = cloned_image[x][y];

  // Can't dilate white pixel
  if (pixel)
    return 0;

  for (int i = 0; i < SE_SIZE; i++)
  {
    for (int j = 0; j < SE_SIZE; j++)
    {
      // Do nothing if SE pixel is 0 or middle pixel
      if (SE[i][j])
        continue;

      // SE pixel is 1
      int x_check = x - SE_HALF_SIZE + i;
      int y_check = y - SE_HALF_SIZE + j;

      // Ignore pixels outside image
      if (x_check < 0 || x_check >= BMP_WIDTH || y_check < 0 || y_check >= BMP_HEIGHT)
        continue;

      // Check if matching pixel in image is 0
      if (!cloned_image[x_check][y_check])
        continue;

      // Found a white pixel. Dilate original pixel
      processed_image[x][y] = 1;
      return 1;
    }
  }
  // Didn't dilate anything
  return 0;
}

int dilate(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  int dilated = 0;

  // Copy image before eroding processed_image
  unsigned char cloned_image[BMP_WIDTH][BMP_HEIGHT];
  memcpy(cloned_image, processed_image, sizeof(cloned_image));

  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      dilated += dilate_pixel(x, y, processed_image, cloned_image);
    }
  }
  return dilated;
}

void open(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  erode(processed_image);
  dilate(processed_image);
}

void close(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  dilate(processed_image);
  erode(processed_image);
  erode(processed_image);
}

void remove_cell(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], int x0, int y0)
{
  for (int x = 0; x < FILL_CIRCLE_SIZE; x++)
  {
    // Skip if outside image
    if (x0 - FILL_CIRCLE_HALF + x < 0 || x0 - FILL_CIRCLE_HALF + x >= BMP_WIDTH)
      continue;

    for (int y = 0; y < FILL_CIRCLE_SIZE + 2; y++)
    {
      // Skip if outside
      if (y0 - FILL_CIRCLE_HALF + y < 0 || y0 - FILL_CIRCLE_HALF + y >= BMP_HEIGHT)
        continue;

      // Only remove cells in the detection zone
      if (filled_circle[x][y])
      {
        processed_image[x0 - FILL_CIRCLE_HALF + x][y0 - FILL_CIRCLE_HALF + y] = 0;
      }
    }
  }
}

int detect_around(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], int x0, int y0)
{
  int outside = 0;
  for (int x = x0 - FILTER_ZONE_HALF; x < x0 + FILTER_ZONE_HALF + 1; x++)
  {
    for (int y = y0 - FILTER_ZONE_HALF; y < y0 + FILTER_ZONE_HALF + 1; y++)
    {
      // Skip if outside image
      if (x < 0 || x >= BMP_WIDTH)
        continue;
      // Skip if outside image
      if (y < 0 || y >= BMP_HEIGHT)
        continue;

      if (processed_image[x][y] && filter_zone[x + FILTER_ZONE_HALF - x0][y + FILTER_ZONE_HALF - y0])
        outside++;
    }
  }
  return outside;
  // int around = 0;
  // for (int x = x0 - PRECISION_HALF - 1; x < x0 + PRECISION_HALF + 1; x++)
  // {
  //   // Skip if outside image
  //   if (x < 0 || x >= BMP_WIDTH)
  //     continue;

  //   if (!(y0 - PRECISION_HALF < 0) && processed_image[x][y0 - PRECISION_HALF])
  //   {
  //     around++;
  //   }

  //   if (!(y0 + PRECISION_HALF >= BMP_HEIGHT) && processed_image[x][y0 + PRECISION_HALF])
  //   {
  //     around++;
  //   }
  // }
  // for (int y = y0 - PRECISION_HALF - 1; y < y0 + PRECISION_HALF + 1; y++)
  // {
  //   // Skip if outside image
  //   if (y < 0 || y >= BMP_HEIGHT)
  //     continue;

  //   if (!(x0 - PRECISION_HALF < 0) && processed_image[x0 - PRECISION_HALF][y])
  //     around++;

  //   if (!(x0 + PRECISION_HALF >= BMP_WIDTH) && processed_image[x0 + PRECISION_HALF][y])
  //     around++;
  // }
  // return around;
}
int count_inside(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT], int x0, int y0)
{
  int inside = 0;
  for (int x = x0 - PRECISION_HALF; x < x0 + PRECISION_HALF + 1; x++)
  {
    for (int y = y0 - PRECISION_HALF; y < y0 + PRECISION_HALF + 1; y++)
    {
      // Skip if outside image
      if (x < 0 || x >= BMP_WIDTH)
        continue;
      // Skip if outside image
      if (y < 0 || y >= BMP_HEIGHT)
        continue;

      if (processed_image[x][y] && detection_zone[x + PRECISION_HALF - x0][y + PRECISION_HALF - y0])
        inside++;
    }
  }
  return inside;
}

void detect(unsigned char processed_image[BMP_WIDTH][BMP_HEIGHT])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      if (processed_image[x][y])
      {
        int surrounding_cells = detect_around(processed_image, x, y);
        int inside_cells = count_inside(processed_image, x, y);
        if (surrounding_cells && inside_cells / surrounding_cells < RATIO_INSIDE_OUT || inside_cells < MIN_INSIDE_CELLS)
        {
          continue;
        }
        else
        {
          remove_cell(processed_image, x, y);
          cell_positions[0][0] += 1;
          cell_positions[cell_positions[0][0]][0] = x;
          cell_positions[cell_positions[0][0]][1] = y;

          // // Save image after each step of erosion
          // if (cell_positions[0][0] > 10)
          //   continue;
          if (print_cell_detection)
          {
            printf("Cell #%d detected at (%d, %d)\n", cell_positions[0][0], x, y);
            char filename[100];
            greyscale_to_rgb(processed_image, output_image);
            snprintf(filename, 100, "detection_%d_out.bmp", cell_positions[0][0]);
            write_bitmap(output_image, filename);
          }
        }
      }
    }
  }
}
void draw_heart(unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x0, int y0)
{
  // hardcoded fra hjertet ovenover
  for (int x = 0; x < 13; x++)
  {
    for (int y = 0; y < 12; y++)
    {
      int global_x = x0 + x - 7;
      int global_y = y0 + y - 6;

      if (heart[y][x])
      {
        if (global_y < 0 || global_y >= BMP_WIDTH)
          continue;

        if (global_x < 0 || global_x >= BMP_WIDTH)
          continue;

        switch (heart[y][x])
        {
        case 1:
          output_image[global_x][global_y][0] = 255;
          output_image[global_x][global_y][1] = 0;
          output_image[global_x][global_y][2] = 0;
          break;
        case 2:
          output_image[global_x][global_y][0] = 0;
          output_image[global_x][global_y][1] = 0;
          output_image[global_x][global_y][2] = 0;
          break;
        case 3:
          output_image[global_x][global_y][0] = 255;
          output_image[global_x][global_y][1] = 255;
          output_image[global_x][global_y][2] = 255;
          break;
        case 4:
          output_image[global_x][global_y][0] = 140;
          output_image[global_x][global_y][1] = 25;
          output_image[global_x][global_y][2] = 35;
        default:
          break;
        }
      }
    }
  }
}

void draw_x(unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x0, int y0)
{
  for (int x = -5; x < 5; x++)
  {
    int global_x = x0 + x;
    if (global_x < 0 || global_x >= BMP_WIDTH)
      continue;

    for (int y = -5; y < 5; y++)
    {
      int global_y = y0 + y;
      if (global_y < 0 || global_y >= BMP_WIDTH)
        continue;

      output_image[global_x][global_y][0] = 255;
      output_image[global_x][global_y][1] = 128;
      output_image[global_x][global_y][2] = 0;
    }
  }
}

void cell_detection(char *input_path, char *output_path, char print_steps)
{
  // Load image from file
  read_bitmap(input_path, input_image);
  rgb_to_greyscale(input_image, processed_image);

  // Do stuff
  unsigned char threshold = get_otsu_threshold(processed_image) + THRESHOLD_OFFSET;
  printf("Threshold: %d\n", threshold); // TODO: Remove
  apply_threshold(processed_image, threshold);

  int steps = 0;

  // Save image before erosion
  greyscale_to_rgb(processed_image, output_image);
  char filename[15];
  strcpy(filename, "step_0_out.bmp");
  write_bitmap(output_image, filename);

  while (erode(processed_image))
  {
    // Save image after each step of erosion
    if (print_steps)
    {
      char filename[100];
      greyscale_to_rgb(processed_image, output_image);
      snprintf(filename, 100, "step_%d_out.bmp", steps);
      write_bitmap(output_image, filename);
      steps++;
    }

    // Detect cells
    detect(processed_image);
  }

  // Save image to file

  for (int i = 1; i <= cell_positions[0][0]; i++)
  {
    draw_heart(input_image, cell_positions[i][0], cell_positions[i][1]);
  }
  write_bitmap(input_image, output_path);

  printf("%d cells found \n", cell_positions[0][0]);
}

void benchmark()
{
  // Array to store all the file paths
  char args[35][50] = {
      // 10 Easy samples
      "./samples/easy/1EASY.bmp",
      "./samples/easy/2EASY.bmp",
      "./samples/easy/3EASY.bmp",
      "./samples/easy/4EASY.bmp",
      "./samples/easy/5EASY.bmp",
      "./samples/easy/6EASY.bmp",
      "./samples/easy/7EASY.bmp",
      "./samples/easy/8EASY.bmp",
      "./samples/easy/9EASY.bmp",
      "./samples/easy/10EASY.bmp",

      // 10 Medium samples
      "./samples/medium/1MEDIUM.bmp",
      "./samples/medium/2MEDIUM.bmp",
      "./samples/medium/3MEDIUM.bmp",
      "./samples/medium/4MEDIUM.bmp",
      "./samples/medium/5MEDIUM.bmp",
      "./samples/medium/6MEDIUM.bmp",
      "./samples/medium/7MEDIUM.bmp",
      "./samples/medium/8MEDIUM.bmp",
      "./samples/medium/9MEDIUM.bmp",
      "./samples/medium/10MEDIUM.bmp",

      // 10 Hard samples
      "./samples/hard/1HARD.bmp",
      "./samples/hard/2HARD.bmp",
      "./samples/hard/3HARD.bmp",
      "./samples/hard/4HARD.bmp",
      "./samples/hard/5HARD.bmp",
      "./samples/hard/6HARD.bmp",
      "./samples/hard/7HARD.bmp",
      "./samples/hard/8HARD.bmp",
      "./samples/hard/9HARD.bmp",
      "./samples/hard/10HARD.bmp",

      // 5 Impossible samples
      "./samples/impossible/1IMPOSSIBLE.bmp",
      "./samples/impossible/2IMPOSSIBLE.bmp",
      "./samples/impossible/3IMPOSSIBLE.bmp",
      "./samples/impossible/4IMPOSSIBLE.bmp",
      "./samples/impossible/5IMPOSSIBLE.bmp"};

  int sucesses = 0;
  int missed_cells = 0;
  for (int i = 0; i < 35; i++)
  {
    cell_positions[0][0] = 0;
    printf("Processing file: %s\n", args[i]);

    char output_path[100];
    snprintf(output_path, 100, "%d_out.bmp", i + 1);

    cell_detection(args[i], output_path, 0);
    missed_cells += abs(300 - cell_positions[0][0]);
    if (cell_positions[0][0] < 280 || cell_positions[0][0] > 320)
    {
      printf("FAILED, %d \n\n\n", 300 - cell_positions[0][0]);
    }
    else
    {
      printf("PASSED \n\n\n");
      sucesses++;
    }
  }
  printf("Total %d/35\n", sucesses);
  printf("Total missed cells: %d", missed_cells);
}

// Main function
int main(int argc, char **argv)
{
  // argc counts how may arguments are passed
  // argv[0] is a string with the name of the program
  // argv[1] is the first command line argument (input image)
  // argv[2] is the second command line argument (output image)

  // Checking that 2 arguments are passed
  if (argc == 2)
  {
    benchmark();
    exit(0);
  }

  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  printf("Example program - 02132 - A1\n");
  cell_detection(argv[1], argv[2], 1);

  return 0;
}
