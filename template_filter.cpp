#include <ap_fixed.h>
#include <ap_int.h>
#include <stdint.h>
#include <assert.h>

#define SIZEN 20

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


void template_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, int y_position[SIZEN],int x_position[SIZEN],int size[SIZEN]){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE m_axi depth=11 port=x_position offset=slave
#pragma HLS INTERFACE m_axi depth=11 port=y_position offset=slave
#pragma HLS INTERFACE m_axi depth=11 port=size offset=slave
#pragma HLS INTERFACE s_axilite port=w
#pragma HLS INTERFACE s_axilite port=h

#pragma HLS INTERFACE m_axi depth=2073600 port=in_data offset=slave // This will NOT work for resolutions higher than 1080p
#pragma HLS INTERFACE m_axi depth=2073600 port=out_data offset=slave



	for (int i = 0; i < h; ++i) {

		int last_gray = 0;

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

			int y_val = y_position[1];
			int x_val = x_position[1];
			bool inside = false;

			for (int object_k = 0; object_k < 11; object_k++){
				if(j > x_position[object_k] and j < x_position[object_k]+size[object_k] and i > y_position[object_k] and i < y_position[object_k]+size[object_k]){
					inside = true;
				}
			}

			if (inside){//and i > y_val and i < y_val+size
				float curr_gray = 0.2126f*in_r  + 0.7152f*in_g  + 0.0722f*in_b ;
				int Y= abs(int(curr_gray)-last_gray);
				last_gray = curr_gray;

				out_r = Y;
				out_b = Y;
				out_g = Y;

			}
			else{
				out_r = in_r;
				out_g = in_g;
				out_b = in_b;
			}

			unsigned int output = out_r | (out_g << 8) | (out_b << 16);
			*out_data++ = output;

		}

	}

}
