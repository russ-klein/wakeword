
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "defines.h"
#include "read_wave.h"
#include "mfcc.h"
#include "ref_mfccs.h"
#include "weights.h"
#include "catapult_accel.h"
#include "cat_access.h"
#include "conv_2d.h"
#include "dense.h"


//=====Keras layer functions=================//


void softmax(
             float *predictions,
             float *probabilities,
             int count)
{
    int i;
    double sum;
    double f;

    sum = 0.0;

    for (i=0; i<count; i++) {
        f = predictions[i];
        sum += exp(f);
    }

    for (i=0; i<count; i++) {
        probabilities[i] = exp(predictions[i])/sum;
    }
}

void load_memory(float *memory)
{

#ifdef HOST     // only neccesary when running on the host, when embedded the weights will be loaded into memory
    size_t r;
    FILE *weight_database;
    char *weight_path; 
    char weight_base_filename[] = "weights_float.bin";
    char weight_filename[10240];

    weight_path = getenv("WEIGHT_PATH");

    if (weight_path) sprintf(weight_filename, "%s/%s", weight_path, weight_base_filename);
    else strcpy(weight_filename, weight_base_filename);

    weight_database = fopen(weight_filename, "r");

    if (weight_database == NULL) {
        fprintf(stderr, "Unable to open file '%s' for reading \n", weight_filename);
        perror(program_name);
        exit(0);
    }

    r = fread(memory, sizeof(float), size_of_weights, weight_database);

    if (r != size_of_weights) {
        fprintf(stderr, "Unable to read in weights from file '%s' \n", weight_filename);
        perror(program_name);
        exit(0);
    }

    fclose(weight_database);

#endif
}


#include "auto_infer.c"

void sw_inference(float *input_image, float *memory, float *probabilities)
{
    int image_offset = size_of_weights;
    int i;
    const int chatty = 0;

    load_memory(memory);
    memcpy(memory+image_offset, input_image, image_height * image_width * 1 * sizeof(float));

    sw_auto_infer(memory, image_offset, probabilities);

    if (chatty) {
        printf("sw prediction: \n");
        for (i=0; i<10; i++) {
           printf("%d = %8.6f \n", i, probabilities[i]);
        }
        printf("\n");
    }
}


void hw_inference(float *input_image, cat_memory_type *memory, float *probabilities)
{
    int image_offset = size_of_weights;
    int i;
    const int chatty = 0;

    load_cat_memory(memory);
    copy_to_cat(memory, image_offset, input_image, image_height * image_width * 1); // layer1_input_images);

    hw_auto_infer(memory, image_offset, probabilities);

    if (chatty) {
        printf("hw prediction: \n");
        for (i=0; i<10; i++) {
           printf("%d = %8.6f \n", i, probabilities[i]);
        }
        printf("\n");
    }
}


int not_close(float a, float b)
{
    if (a > b) {
       if ((a - b) > 0.001) return 1; else return 0;
    }

    if (b > a) {
       if ((b - a) > 0.001) return 1; else return 0;
    }

    return 0;
}


#ifdef HOST
#define MAX_ERRORS 10

void check_mfcc(int test_id, float *spec, float *ref)
{
    int i;
    int errors = 0;

    for (i=0; i<SPECTROGRAM_SIZE; i++) {
       if (not_close(spec[i], ref[i])) {
          fprintf(stderr, "index %d does not match, expected %13.8f, found %13.8f \n", i, ref[i], spec[i]);
          errors++;
       }
       if (errors >= MAX_ERRORS) {
          fprintf(stderr, "too many mismatches\n");
          return;
       }
    }
    if (errors > 0) {
       fprintf(stderr, "check mfcc %d failed: %d errors \n", test_id, errors);
    } else {
       fprintf(stderr, "check mfcc %d passed! \n", test_id);
    }
} 


struct test_data_struct {
    char *filename;
    float *expected_results;
};

typedef struct test_data_struct mfcc_testdata_type;

void test_mfcc()
{
    float waveform[SAMPLE_SIZE];
    float spectrogram[SPECTROGRAM_SIZE];
    mfcc_testdata_type mfcc_testdata[] = {
            { (char *) "../testdata/zero.wav",  (float *) mfcc_zero_values  },
            { (char *) "../testdata/one.wav",   (float *) mfcc_one_values   },
            { (char *) "../testdata/two.wav",   (float *) mfcc_two_values   },
            { (char *) "../testdata/three.wav", (float *) mfcc_three_values },
            { (char *) "../testdata/four.wav",  (float *) mfcc_four_values  },
            { (char *) "../testdata/five.wav",  (float *) mfcc_five_values  },
            { (char *) "../testdata/six.wav",   (float *) mfcc_six_values   },
            { (char *) "../testdata/seven.wav", (float *) mfcc_seven_values },
            { (char *) "../testdata/eight.wav", (float *) mfcc_eight_values },
            { (char *) "../testdata/nine.wav",  (float *) mfcc_nine_values  }
    };

    const int num_tests = sizeof(mfcc_testdata)/sizeof(mfcc_testdata[0]);
    int i;

    for (i=0; i<num_tests; i++) {
       read_wavefile(mfcc_testdata[i].filename, waveform);
       mfcc(waveform, spectrogram);
       check_mfcc(i, spectrogram, mfcc_testdata[i].expected_results);
    } 
}

struct features_and_labels_struct {
    float *features;
    int   label;
};

typedef struct features_and_labels_struct testdata_type;

void test_inference()
{
    float spectrogram[SPECTROGRAM_SIZE];
    static float           sw_memory[0x1000000];
    static cat_memory_type hw_memory[0x1000000];  // make it static so you do not blow up the stack
    float sw_prob[10];
    float hw_prob[10];
    int i;
    const testdata_type   testdata[] = 
                                 { { (float *) mfcc_zero_values,  0},
                                   { (float *) mfcc_one_values,   1},
                                   { (float *) mfcc_two_values,   2},
                                   { (float *) mfcc_three_values, 3},
                                   { (float *) mfcc_four_values,  4},
                                   { (float *) mfcc_five_values,  5},
                                   { (float *) mfcc_six_values,   6},
                                   { (float *) mfcc_seven_values, 7},
                                   { (float *) mfcc_eight_values, 8},
                                   { (float *) mfcc_nine_values,  9} };

    const int num_tests = sizeof(testdata)/sizeof(testdata[0]);

    printf("testing sw inference... \n");

    for (i=0; i<num_tests; i++) {
       memcpy(spectrogram, testdata[i].features, SPECTROGRAM_SIZE * sizeof(float));
       sw_inference(spectrogram, sw_memory, sw_prob);
       if (sw_prob[testdata[i].label] > 0.5) printf("sw inference for %d passed %6.2f \n", i, sw_prob[i] * 100.0); 
                                        else printf("sw inference for %d failed %6.2f \n", i, sw_prob[i] * 100.0);
    }

    printf("testing hw inference... \n");

    for (i=0; i<num_tests; i++) {
       memcpy(spectrogram, testdata[i].features, SPECTROGRAM_SIZE * sizeof(float));
       hw_inference(spectrogram, hw_memory, hw_prob);
       if (hw_prob[testdata[i].label] > 0.5) printf("hw inference for %d passed %6.2f \n", i, hw_prob[i] * 100.0); 
                        else printf("hw inference for %d failed %6.2f \n", i, hw_prob[i] * 100.0);
    }
}

struct wakeword_testdata_struct {
    char *filename;
    int   label;
};

typedef struct wakeword_testdata_struct wakeword_testdata_type;

void test_wakeword()
{
    float spectrogram[SPECTROGRAM_SIZE];
    static float           sw_memory[0x1000000];
    static cat_memory_type hw_memory[0x1000000];  // make it static so you do not blow up the stack
    float sw_prob[10];
    float hw_prob[10];
    int i;
#ifdef HOST
    wakeword_testdata_type wakeword_testdata[] = {
            { (char *) "../testdata/zero.wav",  0 },
            { (char *) "../testdata/one.wav",   1 },
            { (char *) "../testdata/two.wav",   2 },
            { (char *) "../testdata/three.wav", 3 },
            { (char *) "../testdata/four.wav",  4 },
            { (char *) "../testdata/five.wav",  5 },
            { (char *) "../testdata/six.wav",   6 },
            { (char *) "../testdata/seven.wav", 7 },
            { (char *) "../testdata/eight.wav", 8 },
            { (char *) "../testdata/nine.wav",  9 }
    };

    const int num_tests = sizeof(wakeword_testdata)/sizeof(wakeword_testdata[0]);

    float waveform[SAMPLE_SIZE];

#else // not HOST (i.e. RISC-V bare metal 

    const int num_tests = 10;

    float *waveform_data = (float *) 0x40000000;
    float *waveform;
 
#endif


    printf("testing wakeword sw algorithm... \n");

    for (i=0; i<num_tests; i++) {
#ifdef HOST
       read_wavefile(wakeword_testdata[i].filename, waveform);
#else
       waveform = waveform_data + (SAMPLE_SIZE * i);
#endif 
       mfcc(waveform, spectrogram);
       sw_inference(spectrogram, sw_memory, sw_prob);
       if (sw_prob[wakeword_testdata[i].label] > 0.5) printf("sw inference for %d passed %6.2f \n", i, sw_prob[i] * 100.0); 
                                                 else printf("sw inference for %d failed %6.2f \n", i, sw_prob[i] * 100.0);
    }

#ifdef HOST
    printf("testing wakeword hw algorithm... \n");

    for (i=0; i<num_tests; i++) {
       read_wavefile(wakeword_testdata[i].filename, waveform);
       mfcc(waveform, spectrogram);
       hw_inference(spectrogram, hw_memory, hw_prob);
       if (hw_prob[wakeword_testdata[i].label] > 0.5) printf("hw inference for %d passed %6.2f \n", i, hw_prob[i] * 100.0); 
                                                 else printf("hw inference for %d failed %6.2f \n", i, hw_prob[i] * 100.0);
    }
#endif
}

int p_index(float *probability)
{
    int i;
    int max_index = 0;
    float max = 0.0;

    for (i=0; i<10; i++) {
       if (probability[i] > max) {
          max = probability[i];
          max_index = i;
       }
    }
    return max_index;
}

void big_test()
{
    float spectrogram[SPECTROGRAM_SIZE];
    static float           sw_memory[0x1000000];
    static cat_memory_type hw_memory[0x1000000];  // make it static so you do not blow up the stack
    float sw_prob[10];
    float hw_prob[10];
    int i;
    int errors;

    wakeword_testdata_type wakeword_testdata[] = {
     #include "test_files.h"
    };

    const int num_tests = sizeof(wakeword_testdata)/sizeof(wakeword_testdata[0]);

    float waveform[SAMPLE_SIZE];

    printf("testing wakeword sw algorithm... \n");

    errors = 0;
    for (i=0; i<num_tests; i++) {
       printf("file: %s, label: %d percentage: %6.4f \n", wakeword_testdata[i].filename, wakeword_testdata[i].label, 100.0*(i-errors)/(float(i)));
       read_wavefile(wakeword_testdata[i].filename, waveform);
       mfcc(waveform, spectrogram);
       hw_inference(spectrogram, hw_memory, sw_prob);
       //if (sw_prob[wakeword_testdata[i].label] < 0.5) {
       if (wakeword_testdata[i].label != p_index(sw_prob)) {
          errors++;
          printf("sw inference for %s failed %6.2f \n", wakeword_testdata[i].filename, sw_prob[wakeword_testdata[i].label] * 100.0); 
          for (int j=0; j<10; j++) printf("%6.4f ", sw_prob[j]); printf("\n");
       }
    }

    printf("overall accuracy: %d correct of %d tests  percentage: %2.6f \n", num_tests-errors, num_tests, 100.0*(num_tests-errors)/num_tests);
}
#endif // HOST


int main()
{
    char *wave_filename = (char *) "../testdata/three.wav";
    float sw_prob[10];
    float hw_prob[10];
    int errors = 0;
    int i;

    float waveform[SAMPLE_SIZE];
    float spectrogram[SPECTROGRAM_SIZE];

#ifdef HOST
    static cat_memory_type hw_memory[0x1000000];  // make it static so you do not blow up the stack
    static float           sw_memory[0x1000000];
#else
    cat_memory_type *hw_memory = (cat_memory_type *) 0x40000000;
    float           *sw_memory = (float *) 0x50000000;
#endif

#ifdef HOST
    if (!read_wavefile(wave_filename, waveform)) return 1;

    // sweep();
    // test_conv2d();
    // test_dense();
    // all_digits(sw_memory);
    // test_2_conv2d();
    // test_mfcc();
    // test_inference();
    test_wakeword();
    // big_test();
#endif
    mfcc(waveform, spectrogram);

    printf("start sw: \n");
    sw_inference(spectrogram, sw_memory, sw_prob);

    printf("start hw: \n");
    hw_inference(spectrogram, hw_memory, hw_prob);

    for (i=0; i<10; i++) {
        if (not_close(sw_prob[i], hw_prob[i])) {
           printf("%d: hw: %f sw: %f \n", i, hw_prob[i], sw_prob[i]);
           errors++;
        }
    }

    record_differences(sw_memory, hw_memory, size_of_outputs);

    if (errors) {
        printf("Test failed, hw does not match sw! \n");
        return 1;
    } else {
        printf("Test passed! \n");
        return 0;
    }

    return 0;
}

