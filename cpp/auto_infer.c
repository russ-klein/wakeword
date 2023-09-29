  
void sw_auto_infer(float *memory, int image_offset, float *probabilities) 
{ 
 
   conv2d_sw( 
       memory + 2305916,       // offset of input images 
       memory + 0,             // offset of weights 
       memory + 26040,         // offset of biases 
       memory + 2307936,       // offset of output images 
       1,                      // number of input images 
       186,                    // number of output images 
       101,                    // height 
       20,                     // width 
       7,                      // kernel height 
       20,                     // kernel width 
       1,                      // apply relu 
       1);                     // apply bias 
 
   dense_sw( 
       memory + 2307936,       // offset of input images 
       memory + 26226,         // offset of weights 
       memory + 2287986,       // offset of biases 
       memory + 2325606,       // offset of output images 
       1,                      // number of rows in input image 
       17670,                  // number of cols in input image 
       128,                    // number of output images 
       0,                      // don't apply relu 
       1);                     // apply bias 
 
   dense_sw( 
       memory + 2325606,       // offset of input images 
       memory + 2288114,       // offset of weights 
       memory + 2304498,       // offset of biases 
       memory + 2325734,       // offset of output images 
       1,                      // number of rows in input image 
       128,                    // number of cols in input image 
       128,                    // number of output images 
       0,                      // don't apply relu 
       1);                     // apply bias 
 
   dense_sw( 
       memory + 2325734,       // offset of input images 
       memory + 2304626,       // offset of weights 
       memory + 2305906,       // offset of biases 
       memory + 2325862,       // offset of output images 
       1,                      // number of rows in input image 
       128,                    // number of cols in input image 
       10,                     // number of output images 
       0,                      // don't apply relu 
       1);                     // apply bias 
 
   softmax(memory + 2325862, memory + 2325872, 10); 
 
   memcpy(probabilities, memory + 2325872, 10 * sizeof(float)); 
} 
 
void hw_auto_infer(cat_memory_type *memory, int image_offset, float *probabilities) 
{ 
 
   conv2d_hw( 
       memory,                                    
       2305916,                // offset of input images  
       0,                      // offset of weights       
       26040,                  // offset of biases        
       2307936,                // offset of output images 
       1,                      // number of input images  
       186,                    // number of output images 
       101,                    // height                  
       20,                     // width                   
       7,                      // kernel height           
       20,                     // kernel width            
       1,                      // apply relu              
       1);                     // apply bias              
 
   dense_hw( 
       memory,                                           
       2307936,                // offset of input images         
       26226,                  // offset of weights              
       2287986,                // offset of biases               
       2325606,                // offset of output images        
       1,                      // number of rows in input images 
       17670,                  // number of cols in input images 
       128,                    // number of output images        
       0,                      // don't apply relu        
       1);                     // apply bias              
 
   dense_hw( 
       memory,                                           
       2325606,                // offset of input images         
       2288114,                // offset of weights              
       2304498,                // offset of biases               
       2325734,                // offset of output images        
       1,                      // number of rows in input images 
       128,                    // number of cols in input images 
       128,                    // number of output images        
       0,                      // don't apply relu        
       1);                     // apply bias              
 
   dense_hw( 
       memory,                                           
       2325734,                // offset of input images         
       2304626,                // offset of weights              
       2305906,                // offset of biases               
       2325862,                // offset of output images        
       1,                      // number of rows in input images 
       128,                    // number of cols in input images 
       10,                     // number of output images        
       0,                      // don't apply relu        
       1);                     // apply bias              
 
   float softmax_in[10];                        
   float softmax_out[10];                       
 
   copy_from_cat(memory, softmax_in, 2325862, 10); 
 
   softmax(softmax_in, softmax_out, 10);         
 
   memcpy(probabilities, softmax_out, 10 * sizeof(float)); 
} 
 
