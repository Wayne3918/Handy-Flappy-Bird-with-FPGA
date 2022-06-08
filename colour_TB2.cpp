#include <ap_fixed.h>
#include <ap_int.h>
#include <cassert>
#include <iostream>
#include <string>
#include <cstdlib>

#include <hls_opencv.h>

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


void template_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, short& w, short& h, short& x_width, long long int& x_pillar, short& s_bird, long long int& y_depth, short& y_bird, short v_horizontal, short interval, int* ptr, unsigned char& r_avg, unsigned char& g_avg, unsigned char& b_avg, unsigned char  case_t, unsigned char error);


int main() {
	std::cout<<"enter "<<std::endl;


		//Make sure that these much input image resolution
		short w = 1280;
		short h = 720;
		short x_width=60;
		long long int x_pillar = 0x2030203020304030;
		long long int y_depth = 0x2030203020304030;
		short s_bird = 30;
		short y_bird =360;
		short v_horizontal = 50;
		 short interval =400;
		unsigned char r_avg =162;
		unsigned char g_avg=111;
		unsigned char b_avg =97;
		unsigned char error =30;
		unsigned char  case_t = 20;
		int* ptr;
		std::cout<< w <<std::endl;
		int a=0;
		ptr = &a;
		std::cout<< *ptr <<std::endl;
		
		//TEST case
		//Specify Input Image Absolute Path
		cv::Mat src_hls = cv::imread("/home/yf4418/eie-project19/img/WINA.jpg", CV_LOAD_IMAGE_UNCHANGED);
		std::cout << "Image type: " << src_hls.type() << ", no. of channels: " << src_hls.channels() << std::endl;
		//src_hls.convertTo(src_hls, CV_8UC3);
		//cv::cvtColor(src_hls, src_hls, CV_BGR2RGBA);

		std::cout<<"SIZE  "<<src_hls.size()<<std::endl;
		uchar *data_p = src_hls.data;

		uchar *image = (uchar *)malloc(w*h*4);

		for (int i=0; i<w*h; i++){
			image[4*i + 0] = data_p[3*i + 2]; //R - R
			image[4*i + 1] = data_p[3*i + 1]; // B - B
			image[4*i + 2] = data_p[3*i + 0]; // G - G
			image[4*i + 3] = 0;
		}


		unsigned char case_t = 2;
		//void template_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, short& w, short& h, short& x_width, long long int& x_pillar, short& s_bird, long long int& y_depth, short& y_bird, unsigned char s2, char v_horizontal, short interval, int* ptr, unsigned char r_avg, unsigned char g_avg, unsigned char b_avg, unsigned char  case_t){
		template_filter((volatile uint32_t*) image, (volatile uint32_t*) image,  w,  h, x_width, x_pillar,  s_bird, y_depth, y_bird, v_horizontal, interval, ptr, r_avg, g_avg, b_avg,  case_t, error);

		for (int i=0; i<w*h; i++){
			data_p[3*i + 2] = image[4*i + 0];
			data_p[3*i + 1] = image[4*i + 1];
			data_p[3*i + 0] = image[4*i + 2];
		}

		//Specify an Absolute Path for Storing Output Image
		cv::imwrite("/home/yf4418/eie-project19/img/TST.jpg",src_hls);

		free(image);
		
		std::cout << "short x_width=" << x_width << ";\n" << "long long int x_pillar = " << x_pillar << ";\n" << "long long int y_depth = " << y_depth << ";\n" << "short s_bird = " <<s_bird << ";\n" << "short y_bird ="<< y_bird  << ";\n" << "short v_horizontal = " << v_horizontal << ";\n" << " short interval =" << interval << ";\n" << "unsigned char r_avg ="  << (int)r_avg << ";\n";
		std::cout << "unsigned char g_avg=" << (int)g_avg << ";\n" << "unsigned char b_avg =" << (int)b_avg << ";\n" << "unsigned char error =" << error << ";\n" << "unsigned char  case_t = " << (int)case_t << std::endl;


		return 0;

	}