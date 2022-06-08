#include <ap_fixed.h>
#include <ap_int.h>
#include <stdint.h>
#include <assert.h>
//#include <hls_opencv.h>
//#include <iostream>//test
#define SLICE 36
#define COUNT 8
#define BIT 12
#define STARTPOINT 1280
#define CASE_GREEN  30
#define BOUNDARY 1279

typedef ap_uint<8> pixel_type;
typedef ap_int<8> s8b;
typedef ap_uint<8> u8b;
typedef ap_uint<96> u96b;
typedef ap_uint<32> word_32;
typedef ap_ufixed<8,0, AP_RND, AP_SAT> comp_type;
typedef ap_fixed<10,2, AP_RND, AP_SAT> coeff_type;
typedef ap_uint<12> word;
typedef ap_uint<96> package;

struct pixel_data {
	pixel_type blue;
	pixel_type green;
	pixel_type red;
};

//using namespace cv;

void Render(unsigned char& out_r, unsigned char& out_g, unsigned char& out_b, unsigned char& in_r, unsigned char& in_g, unsigned char& in_b, unsigned char& case_i);
//int vertical_location(Mat &img);

void decode_x(package x_pillar, unsigned char x_width, word x_position[COUNT], unsigned char middle);
void encode_x(package& x_pillar, word x_width, word x_position[COUNT],word v_horizontal);
void decode_y(package y_depth, word y_position[COUNT]);
void encode_y(package& y_depth, word y_position[COUNT]);

int vertical_detection(volatile uint32_t* in_data, int w, int h, unsigned char case_t);

void template_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, word& x_width, package& x_pillar, word interval,  word v_horizontal, word& s_bird,  word& y_bird, package& y_depth, word& new_depth,word gap, bool& game_over, unsigned char& r_avg, unsigned char& g_avg, unsigned char& b_avg, unsigned char& case_t, unsigned char error, int& max_h){
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=w
#pragma HLS INTERFACE s_axilite port=h
#pragma HLS INTERFACE m_axi depth=2073600 port=in_data offset=slave // This will NOT work for resolutions higher than 1080p
#pragma HLS INTERFACE m_axi depth=2073600 port=out_data offset=slave

#pragma HLS INTERFACE s_axilite port=x_width bundle=add_io
#pragma HLS INTERFACE s_axilite port=x_pillar bundle=add_io
#pragma HLS INTERFACE s_axilite port=interval bundle=add_io
#pragma HLS INTERFACE s_axilite port=v_horizontal bundle=add_io
#pragma HLS INTERFACE s_axilite port=s_bird bundle=add_io
#pragma HLS INTERFACE s_axilite port=y_bird bundle=add_io
#pragma HLS INTERFACE s_axilite port=y_depth bundle=add_io
#pragma HLS INTERFACE s_axilite port=new_depth bundle=add_io
#pragma HLS INTERFACE s_axilite port=gap bundle=add_io
#pragma HLS INTERFACE s_axilite port=game_over bundle=add_io
#pragma HLS INTERFACE s_axilite port=r_avg bundle=add_io
#pragma HLS INTERFACE s_axilite port=g_avg bundle=add_io
#pragma HLS INTERFACE s_axilite port=b_avg bundle=add_io
#pragma HLS INTERFACE s_axilite port=case_t bundle=add_io
#pragma HLS INTERFACE s_axilite port=error bundle=add_io


	unsigned char case_i = 0;
	unsigned char middle = 100;
	word x_position[8], y_position[8];

	if(case_t < 10){//TEST COLOUR
		int r_sum, g_sum, b_sum;
		template_filter_label4:for (int i = 0; i < h; ++i) {

			template_filter_label5:for (int j = 0; j < w; ++j) {

				#pragma HLS PIPELINE II=1
				#pragma HLS LOOP_FLATTEN off

				unsigned int current = *in_data++;

				unsigned char in_r = current & 0xFF;
				unsigned char in_g = (current >> 8) & 0xFF;
				unsigned char in_b = (current >> 16) & 0xFF;
				unsigned char out_r = 0;
				unsigned char out_b = 0;
				unsigned char out_g = 0;
				word x_bird = 620;

				if (j > x_bird and j < x_bird+30 and i > y_bird and i < y_bird+30){
					if(case_t == 1){
						case_i = 3;//pink
					}else if(case_t == 2){
						case_i = 4;//light green
						r_sum+=in_r;
						g_sum+=in_g;
						b_sum+=in_b;
					}


				}

				Render(out_r,out_g,out_b,in_r,in_g,in_b,case_i);
				 unsigned int output = out_r | (out_g << 8) | (out_b << 16);//bitwise shift
				 *out_data++ = output;
				 case_i = 0;
			}
		}
		word area = s_bird*s_bird;
		r_avg = r_sum/area;
		g_avg = g_sum/area;
		b_avg = b_sum/area;
		//std::cout << r_sum << " " << (int)r_avg << std::endl; //test

	}
	else{//case_t>=10
		int hist[SLICE];
		volatile uint32_t* tmp_img_ptr = in_data;
		int offset = 0;
	  //unsigned char  r_avg = 180,g_avg=120,b_avg=120;
		unsigned char  r_low = 0;
		unsigned char  r_high = 255;
		unsigned char  g_low = 0;
		unsigned char  g_high = 255;
		unsigned char  b_low = 0;
		unsigned char  b_high = 255;
		unsigned char error_g = error;
		if(error>8){
			error_g = error-8;//green less tolerence
		}
		if((255-r_avg)>error){
			 r_high = r_avg + error;
		}
		if(r_avg>error){
			 r_low = r_avg - error;
		}
		if((255-g_avg)>error_g){
			 g_high = g_avg + error_g;
		}
		if(g_avg>error_g){
			 g_low = g_avg - error_g;
		}
		if((255-b_avg)>error){
				b_high = b_avg + error;
		}
		if(b_avg>error){
			 b_low = b_avg - error;
		}

		int max_hist = 0;

		template_filter_label6:for (int hist_index = 0; hist_index < SLICE; hist_index++){//for every intreseted area
			hist[hist_index]=0;
			template_filter_label7:for(int row_i = 0; row_i < h/SLICE; row_i++){//h/SLICE=20 row num
				template_filter_label8:for(int col_i = 620;col_i < 660; col_i++){

					offset = (hist_index*(h/SLICE)+row_i)*w+col_i; //offset = y*w+x , y = hist_index*20+row_i

					//F[] REQUEST COLOUT PIX
					unsigned int current_px = *(tmp_img_ptr + offset);//ptr to px

					unsigned char in_r = current_px & 0xFF;
					unsigned char in_g = (current_px >> 8) & 0xFF;
					unsigned char in_b = (current_px >> 16) & 0xFF;
					unsigned char out_r = 0;
					unsigned char out_g = 0;
					unsigned char out_b = 0;

					//F[] CONSTRUCT HISTOGRAM
					if(in_r > r_low and in_r < r_high and in_g > g_low and in_g < g_high and in_b > b_low and in_b < b_high){
						hist[hist_index]++;
						if(case_t>32){
							out_r = in_r/4*3;
							out_b = 252/4+in_b/4*3;
							out_g = 252/4+in_g/4*3;
							unsigned int output = out_r | (out_g << 8) | (out_b << 16);
							current_px = output;
						}
					}
				}
				//std::cout << "hist index:" << hist_index << "val:" << hist[hist_index] << std::endl;
			}
			if(hist[hist_index]>max_hist and hist_index > 2 and hist_index < 35){
				max_hist = hist_index;
			}
		}
		int max_h_next;
		max_h_next=max_hist*(h/SLICE);

		/*word y_bird_tmp = max_hist*(h/SLICE);
		if(y_bird_tmp > s_bird and y_bird_tmp < h-s_bird){//floor ceiling
			y_bird = y_bird_tmp-s_bird;
		}
		max_h = max_hist;*/

		//pointer to matrix
		/*cv::Size sz(w,h);
		cv::Mat mat1(sz,CV_8UC3, in_data);
		y_bird = vertical_location(mat1);*/
		word y_bird_next;
		y_bird_next = vertical_detection(in_data, w,  h,case_t);

		decode_x(x_pillar, x_width, x_position,middle);
		decode_y(y_depth,y_position);


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

				//assign strip area case
				//word in = 0;
				/*for (word gap_i = 0; gap_i < 5; gap_i++){
					in = gap*gap_i;
					if (j > x_strip+in and j < x_strip+s_bird+in and i > y_strip and i < y_strip+s_bird){
						case_i = 1;
					}
				}*/


				for (int object_k = 0; object_k < 8; object_k++){
					if(j > x_position[object_k] and j < x_position[object_k]+x_width and (i < y_position[object_k] or i > y_position[object_k]+gap)){
						if(case_t>20){
							case_i = (((j-x_position[object_k])%60)/10)+CASE_GREEN;//green
						}else{
							case_i = 4;
						}
					}
				}

				word x_bird = 620;
				//assign bird area case
				if (j > x_bird and j < x_bird+s_bird and i > y_bird and i < y_bird+s_bird){
					case_i = 6;
				}
				if (j > x_bird and j < x_bird+s_bird and i > max_h and i < max_h+s_bird){
					case_i = 2;
				}

				Render(out_r,out_g,out_b,in_r,in_g,in_b,case_i);
				//FAIL TEST
				  if(game_over){
					  out_g = 0;
				  }
				  case_i = 0;
				  unsigned int output = out_r | (out_g << 8) | (out_b << 16);//bitwise shift
				  *out_data++ = output;
			}
		}

				//GAME LOGIC
				if(case_t == 44){
					game_over = true;
				}
				//Collision detection
				if(middle <= 8 and case_t>20){
					if((y_position[middle] > y_bird or y_position[middle]+gap < y_bird+s_bird) and (y_position[middle] > max_h or y_position[middle]+gap < max_h+s_bird)){
						game_over = true;
						case_t = 44;
					}
				}


				//Release new pillar
				bool first_pillar = false;
				u8b pillar_index = 0;
				word smallest_positive = BOUNDARY;
				template_filter_label2:for (s8b i=0; i < COUNT; i++){//smallest positive
					if(x_position[i] > 1 and x_position[i] < smallest_positive){
						smallest_positive = x_position[i];
						//std::cout << "smallest potive" << smallest_positive << std::endl;//test
					}
				}
				template_filter_label3:while(!first_pillar and pillar_index < COUNT){
					if(x_position[pillar_index] == STARTPOINT and smallest_positive >= interval){
						x_position[pillar_index]= 20;//Reserve point
						y_position[pillar_index]=new_depth;
						first_pillar = true;
					}
					pillar_index++;
				}

				encode_x(x_pillar,x_width,x_position,v_horizontal);
				encode_y(y_depth, y_position);
				y_bird = y_bird_next;
				max_h = max_h_next;
			}
}


void Render(unsigned char& out_r, unsigned char& out_g, unsigned char& out_b, unsigned char& in_r, unsigned char& in_g, unsigned char& in_b, unsigned char& case_i){//, unsigned char row, unsigned char col, int* ptr
	switch (case_i)
	{
	 case 1://strip red
		 out_r = 0;
		 out_b = 0;
		 out_g = 200;
		 break;
	 case 2://blue1
		 out_r = 60;
		 out_b = 100;
		 out_g = 50;
		 break;
	 case 3://pink
	 	 out_r = (220 + in_r)/2;
		 out_g = (100 + in_g)/2;
	   	 out_b = (150 + in_b)/2;
		 break;
	case 4://light green
	     out_r = 10+ in_r/2;
		 out_g = (190 + in_g)/2;
		 out_b = 10 + in_b/2;
		 break;
	 case 5://green5
		 out_r = 197;
		 out_g = 245;
		 out_b = 185;
		 break;
	 case 6://pink
		 out_r = 255;
		 out_g = 168;
		 out_b = 211;
		 break;
	 case 30://green1
		 out_r = 40;
		 out_g = 100;
		 out_b = 91;
		 break;
	 case 31://green1
		 out_r = 40;
		 out_g = 128;
		 out_b = 91;
		 break;
	 case 32://green2
		 out_r = 40;
		 out_g = 155;
		 out_b = 91;
		 break;
	 case 33://green3
		 out_r = 60;
		 out_g = 188;
		 out_b = 91;
		 break;
	 case 34://green4
		 out_r = 100;
		 out_g = 200;
		 out_b = 110;
		 break;
	 case 35://green5
		 out_r = 197;
		 out_g = 245;
		 out_b = 185;
		 break;
	 default:
		 out_r = in_r;
		 out_g = in_g;
		 out_b = in_b;
	}

}


void decode_x(package x_pillar, unsigned char x_width, word x_position[COUNT], unsigned char middle){
	decode_x_label0:for(u8b i = 0; i < COUNT; i++){
		package tmp=x_pillar;
		x_position[i] = ((tmp >> i*BIT) & 0xFFF);//assume BIT 12/4=3*F
		//std::cout << i << " of "<< x_position[i] << std::endl;//test
		//GAME LOGIC if pillar pass through middle point, RETURN index
		if(x_position[i]+x_width > 640 and x_position[i] < 700){//assume w 1280 h 720
			middle = i;
		}
	}

	//std::cout << std::endl;//test
}

void encode_x(package& x_pillar, word x_width, word x_position[COUNT],word v_horizontal){
	x_pillar = 0;
	encode_x_label1:for (u8b i = 0; i < COUNT; i++){
		if(x_position[i] > BOUNDARY or x_position[i] == STARTPOINT){//right most assume w 1280
			x_position[i] = STARTPOINT;
		}else{
			x_position[i] += v_horizontal;
		}
		package tmp = x_position[i];
		x_pillar = x_pillar | (tmp << BIT*i);
		//std::cout << std::hex << x_pillar << std::dec << std::endl;//test
	}
}


void decode_y(package y_depth, word y_position[COUNT]){
	decode_y_label2:for(u8b i = 0; i < COUNT; i++){
		package tmp = y_depth;
		y_position[i] = ((tmp >> i*BIT) & 0xFFF);//assume BIT 12/4=3*F
	}
}

void encode_y(package& y_depth, word y_position[COUNT]){
	y_depth = y_position[0]/2;
	encode_y_label3:for(u8b i = 1; i < COUNT; i++){
		package tmp = y_position[i];
		y_depth = y_depth | (tmp << BIT*i);
	}
}


int vertical_detection(volatile uint32_t* in_data, int w, int h, unsigned char case_t){

	int vertical_position = 320;
	int pixel_counter = 1;
	int position;
	vertical_detection_label0:for (int i = 0; i < h; ++i) {

		int last_gray = 0;

		vertical_detection_label1:for (int j = 0; j < w; ++j) {

			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off

			unsigned int current = *in_data++;

			unsigned char in_r = current & 0xFF;
			unsigned char in_g = (current >> 8) & 0xFF;
			unsigned char in_b = (current >> 16) & 0xFF;

			unsigned char out_r = 0;
			unsigned char out_b = 0;
			unsigned char out_g = 0;

			unsigned char min;
			unsigned char max;
			unsigned char rg;


			if(in_r>in_b)
			{
				if(in_r>in_g)
				{
					max = in_r;

				}
			}
			else if(in_b>in_r)
			{
				if(in_b>in_g)
				{
					max = in_b;
				}
			}
			else if(in_g>in_b)
			{
				if(in_g>in_r)
				{
					max = in_g;
				}
			}

			if(in_r<in_b)
			{
				if(in_r<in_g)
				{
					min = in_r;
				}
			}
			else if(in_b<in_r)
			{
				if(in_b<in_g)
				{
					min = in_b;
				}
			}
			else if(in_g<in_b)
			{
				if(in_g<in_r)
				{
					min = in_g;
				}
			}

			if(in_r > in_g)
			{
				rg = in_r - in_g;
			}
			else
			{
				rg = in_g - in_r;
			}

			if ((in_r > 95 && in_g >40 && in_b > 20 && max-min>15 && rg>15 && in_r > in_g && in_r > in_b)||(in_r >220 && in_g>210 && in_b>170 && rg >15 && in_r > in_b && in_g>in_b)){
					out_r = 255*4/5+in_r/5;
					out_b = 105*4/5+in_b/5;
					out_g = 108*4/5+in_g/5;
					vertical_position = vertical_position + j;
					pixel_counter++;
			}

			/*else{
					out_r = 0;
					out_g = 0;
					out_b = 0;
			}*/
			if(case_t < 35){
				unsigned int output = out_r | (out_g << 8) | (out_b << 16);
				current = output;
		  }

		}

	}
	position = vertical_position/pixel_counter;

	return position;

}

/*
 *
void decode_x(package x_pillar, unsigned char x_width, word x_position[COUNT], unsigned char middle){
	word offset_x = 0;
	decode_x_label0:for(int i = 0; i < 8; i++){
		s8b temp = ((x_pillar >> i*8) & 0xFF);
		word doubler = 2*temp;
		x_position[i] = doubler + offset_x;
		offset_x = x_position[i] + x_width;
		if(x_position[i]+x_width > 640 and x_position[i] < 700){
			middle = i;
		}
	}
}

void encode_x(package& x_pillar, word x_width, word x_position[COUNT],word v_horizontal){
	x_pillar = 0; //| (out_g << 8) | (out_b << 16);//bitwise shift
	encode_x_label7:for (s8b i = 0; i < COUNT; i++){
		x_position[i]+=v_horizontal;
	}
	word offset_x;
	encode_x_label1:for (s8b i = 0; i < COUNT; i++){
		if(x_position[i] > 1270){//right most
		 offset_x = -v_horizontal;//initial
		}else if((i == 0 and x_position[7] > x_position[0]) or (x_position[i-1]>x_position[i])){//left most
		 offset_x = x_position[i];
		}else if (i==0){//i = 0
		 offset_x = x_position[0] - x_position[7]-x_width;
		}else{//normal
		 offset_x = x_position[i] - x_position[i-1]-x_width;
		}
		x_pillar = x_pillar | ((offset_x/2)<< 8*i);
	}
}
 */
