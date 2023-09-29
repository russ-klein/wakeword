#ifndef REGIONS_H_INCLUDED 
#define REGIONS_H_INCLUDED 


static unsigned int region_map[][2] = { 
  {          0,      26040 },  // conv2d weights 
  {      26040,        186 },  // conv2d biases 
  {      26226,    2261760 },  // dense weights 
  {    2287986,        128 },  // dense biases 
  {    2288114,      16384 },  // dense_1 weights 
  {    2304498,        128 },  // dense_1 biases 
  {    2304626,       1280 },  // dense_2 weights 
  {    2305906,         10 },  // dense_2 biases 
  {    2305916,       2020 },  // input_image 
  {    2307936,      17670 },  // conv2d outputs 
  {    2325606,        128 },  // dense outputs 
  {    2325734,        128 },  // dense_1 outputs 
  {    2325862,         10 },  // dense_2 outputs 
  {    2325872, 4294967295 }   // out of bounds 
}; 
 
 
static char region_names[][40] = { 
  { "conv2d weights" }, 
  { "conv2d biases " }, 
  { "dense weights" }, 
  { "dense biases " }, 
  { "dense_1 weights" }, 
  { "dense_1 biases " }, 
  { "dense_2 weights" }, 
  { "dense_2 biases " }, 
  { "input image " }, 
  { "conv2d outputs " }, 
  { "dense outputs " }, 
  { "dense_1 outputs " }, 
  { "dense_2 outputs " }, 
  { "out of bounds " } 
}; 

#endif 
