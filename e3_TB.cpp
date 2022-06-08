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
typedef ap_uint<8> u8b;
typedef ap_uint<32> word_32;
typedef ap_ufixed<8,0, AP_RND, AP_SAT> comp_type;
typedef ap_fixed<10,2, AP_RND, AP_SAT> coeff_type;
typedef ap_uint<12> word;
typedef ap_uint<96> package;

#define COUNT 8
#define BIT 12


struct pixel_data {
	pixel_type blue;
	pixel_type green;
	pixel_type red;
};

void encode_x(package& x_pillar, word x_position[COUNT]){
	x_pillar = 0;
	encode_x_label1:for (u8b i = 0; i < COUNT; i++){
		package tmp = x_position[i];
		std::cout <<"tmp="<< std::hex << tmp << std::dec << std::endl;//test
		x_pillar = x_pillar | (tmp << BIT*i);
		std::cout << std::hex << x_pillar << std::dec << std::endl;//test
	}
}


void template_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, word& x_width, package& x_pillar, word interval,  word v_horizontal, word& s_bird,  word& y_bird, package& y_depth, word& new_depth,word gap, bool& game_over, unsigned char& r_avg, unsigned char& g_avg, unsigned char& b_avg, unsigned char& case_t, unsigned char error, int& max_h);


int main() {
	//std::cout<<"enter "<<std::endl;


		//Make sure that these much input image resolution


		//long long int x_pillar = 0x1020102010201020;
		//long long int y_depth = 0x2030203020302030;
		package x_pillar;
		package y_depth;
		word x[COUNT];
		x[0]=0;
		x[1]=200;
		x[2]=300;
		x[3]=400;
		x[4]=600;
		x[5]=800;
		x[6]=1000;
		x[7]=1200;
		encode_x(x_pillar,x);

		word y[COUNT];
		x[0]=0;
		y[1]=100;
		y[2]=200;
		y[3]=300;
		y[4]=400;
		y[5]=500;
		y[6]=600;
		y[7]=700;
		encode_x(y_depth,y);

		int w = 1280;
		int h = 720;

		word x_width=60;
		//package x_pillar = 0x2504705906AA8CA9CF8B9BFA;
		word interval = 100;
		word v_horizontal = 100;
		word s_bird = 30;
		word y_bird =210;
		//package y_depth = 0x0AA0AA0AA0AA0AA0AA0AA0AA;
		word new_depth = 100;
		word gap = 300;

		unsigned char r_avg =162;
		unsigned char g_avg=111;
		unsigned char b_avg =97;
		unsigned char error =30;
		unsigned char  case_t = 34;

		bool game_over = false;
		int max_h=0;

for(int i = 0; i < 9; i++){


	//Specify Input Image Absolute Path
	std::string adr;
	/*adr = "/home/yf4418/eie-project19/img/";
	adr += std::to_string(i);
	adr = adr + ".jpg";*/
	cv::Mat src_hls = cv::imread("/home/yf4418/eie-project19/img/"+std::to_string(i)+".jpg", CV_LOAD_IMAGE_UNCHANGED);
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


	int max_h;
	//void template_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, short& w, short& h, short& x_width, long long int& x_pillar, short& s_bird, long long int& y_depth, short& y_bird, unsigned char s2, char v_horizontal, short interval, int* ptr, unsigned char r_avg, unsigned char g_avg, unsigned char b_avg, unsigned char  case_t){
	template_filter((volatile uint32_t*) image, (volatile uint32_t*) image,  1280,  720, x_width, x_pillar, interval,  v_horizontal, s_bird, y_bird, y_depth, new_depth, gap, game_over , r_avg, g_avg, b_avg,  case_t, error, max_h);

	for (int i=0; i<w*h; i++){
		data_p[3*i + 2] = image[4*i + 0];
		data_p[3*i + 1] = image[4*i + 1];
		data_p[3*i + 0] = image[4*i + 2];
	}

	std::cout << "max_h" <<max_h << std::endl;
	//Specify an Absolute Path for Storing Output Image
	cv::imwrite("/home/yf4418/eie-project19/img/TST"+std::to_string(i)+".jpg",src_hls);
	free(image);
	std::cout << std::endl;
	std::cout << "short x_width=" << x_width << ";\n" << "long long int x_pillar = " << x_pillar << ";\n" << "long long int y_depth = " << y_depth << ";\n" << "short s_bird = " <<s_bird << ";\n" << "short y_bird ="<< y_bird  << ";\n" << "short v_horizontal = " << v_horizontal << ";\n" << " short interval =" << interval << ";\n" << "unsigned char r_avg ="  << (int)r_avg << ";\n";
	std::cout << "unsigned char g_avg=" << (int)g_avg << ";\n" << "unsigned char b_avg =" << (int)b_avg << ";\n" << "unsigned char error =" << (int)error << ";\n" << "unsigned char  case_t = " << (int)case_t << ";\n" << std::endl;

 }

	return 0;

}
