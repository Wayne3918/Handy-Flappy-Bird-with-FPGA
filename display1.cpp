#include <ap_fixed.h>
#include <ap_int.h>
#include <stdint.h>
#include <assert.h>

typedef ap_uint<8> pixel_type;
typedef ap_int<8> pixel_type_s;
typedef ap_uint<96> u96b;
typedef ap_uint<32> word_32;
typedef ap_ufixed<8,0, AP_RND, AP_SAT> comp_type;
typedef ap_fixed<10,2, AP_RND, AP_SAT> coeff_type;

struct pixel_data {
	pixel_type blue;
	pixel_type green;
	pixel_type red;
};


void displayer(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, int x1, int y1, int s1, int x2, int y2, int s2, int interval, int* ptr){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=x1
#pragma HLS INTERFACE s_axilite port=x2
#pragma HLS INTERFACE s_axilite port=y1
#pragma HLS INTERFACE s_axilite port=y2
#pragma HLS INTERFACE s_axilite port=s1
#pragma HLS INTERFACE s_axilite port=s2
#pragma HLS INTERFACE s_axilite port=interval
#pragma HLS INTERFACE s_axilite port=ptr
#pragma HLS INTERFACE s_axilite port=w
#pragma HLS INTERFACE s_axilite port=h

#pragma HLS INTERFACE m_axi depth=2073600 port=in_data offset=slave // This will NOT work for resolutions higher than 1080p
#pragma HLS INTERFACE m_axi depth=2073600 port=out_data offset=slave



	for (int i = 0; i < h; ++i) {

		for (int j = 0; j < w; ++j) {

			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off

			unsigned int current = *in_data++;

			unsigned char in_r = current & 0xFF;
			unsigned char in_g = (current >> 8) & 0xFF;
			unsigned char in_b = (current >> 16) & 0xFF;
			unsigned char out_r = 0;
			unsigned char out_b = 0;
			unsigned char out_g = 0;

			int case_i = 0;
			int in;

			for (int interval_i = 0; interval_i < 5; interval_i++){
				in = interval*interval_i;
				if (j > x1+in and j < x1+s1+in and i > y1 and i < y1+s1){
					case_i = 1;
				}
			}

			if (j > x2+in and j < x2+s2+in and i > y2 and i < y2+s2){
				case_i = 2;
			}

			 switch (case_i)
			 {
			 	case 1://bird
					out_r = 255;
					out_b = 0;
					out_g = 0;
					break;
				case 2://strip
					out_r = 60;
					out_b = 100;
					out_g = 50;
					break;
				default:
					out_r = in_r;
					out_g = in_g;
					out_b = in_b;
			}

		 //test
		 if(*ptr == 1){
			 out_g = 0;
		 }

			unsigned int output = out_r | (out_g << 8) | (out_b << 16);//bitwise shift
			*out_data++ = output;

		}

	}

}
