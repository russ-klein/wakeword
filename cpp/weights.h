#ifndef WEIGHTS_H_INCLUDED 
#define WEIGHTS_H_INCLUDED 


      
   //=======layer 1 - convolution===============================   
      
   static const int layer1_input_images       = 1;  
   static const int layer1_output_images      = 186;  
   static const int layer1_weights_rows       = 20;  
   static const int layer1_weights_cols       = 7;  
      
   static const int layer1_num_weights        = 26040;  
      
   static const int layer1_weight_offset      = 0;  
   static const int layer1_out_size           = 17670;  
      
      
   static const int layer1_num_bias_values    = 186;  
   static const int layer1_bias_offset        = 26040;  
      
      
      
   //=======layer 2 - dense=====================================   
      
   static const int layer2_weights_rows       = 128; 
   static const int layer2_weights_cols       = 17670; 
      
   static const int layer2_num_weights        = 2261760;  
      
   static const int layer2_weight_offset      = 26226;  
   static const int layer2_out_size           = 128;  
      
      
      
   static const int layer2_num_bias_values    = 128;  
   static const int layer2_bias_offset        = 2287986;  
      
      
      
   //=======layer 3 - dense=====================================   
      
   static const int layer3_weights_rows       = 128; 
   static const int layer3_weights_cols       = 128; 
      
   static const int layer3_num_weights        = 16384;  
      
   static const int layer3_weight_offset      = 2288114;  
   static const int layer3_out_size           = 128;  
      
      
      
   static const int layer3_num_bias_values    = 128;  
   static const int layer3_bias_offset        = 2304498;  
      
      
      
   //=======layer 4 - dense=====================================   
      
   static const int layer4_weights_rows       = 10; 
   static const int layer4_weights_cols       = 128; 
      
   static const int layer4_num_weights        = 1280;  
      
   static const int layer4_weight_offset      = 2304626;  
   static const int layer4_out_size           = 10;  
      
      
      
   static const int layer4_num_bias_values    = 10;  
   static const int layer4_bias_offset        = 2305906;  
      
      
 
   //=======End of layers==========================================   
 
 
   static int const image_height              = 101; 
   static int const image_width               = 20; 
   static int const image_size                = 2020; 
   static int const num_images                = 1; 
 
   static int const size_of_weights           = 2305916; 
   static int const size_of_outputs           = 2325872; 
 
 
#endif 
 
