#include <ap_fixed.h>
#include <ap_int.h>
#include <cassert>
#include <iostream>

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


void displayer(volatile uint32_t* in_data, volatile uint32_t* out_data, int w, int h, int x1, int y1, int s1, int x2, int y2, int s2, int interval, int* ptr);


int main() {

	//Specify Input Image Absolute Path
	cv::Mat src_hls = cv::imread("/home/yf4418/eie-project19/img/blue.jpg", CV_LOAD_IMAGE_UNCHANGED);
	std::cout << "Image type: " << src_hls.type() << ", no. of channels: " << src_hls.channels() << std::endl;
	//src_hls.convertTo(src_hls, CV_8UC3);
	//cv::cvtColor(src_hls, src_hls, CV_BGR2RGBA);

	std::cout<<"SIZE  "<<src_hls.size()<<std::endl;
	uchar *data_p = src_hls.data;

	//Make sure that these much input image resolution
	int w = 1920;
	int h = 1080;


	uchar *image = (uchar *)malloc(w*h*4);

	for (int i=0; i<w*h; i++){
		image[4*i + 0] = data_p[3*i + 2]; //R - R
		image[4*i + 1] = data_p[3*i + 1]; // B - B
		image[4*i + 2] = data_p[3*i + 0]; // G - G
		image[4*i + 3] = 0;
	}

	int* ptr;
	*ptr = 0;
	//	template_filter((volatile uint32_t *)image, (volatile uint32_t *)image, w, h, x1, int y1, int s1, int x2, int y2, int s2, int interval, int* ptr);
	displayer((volatile uint32_t *)image, (volatile uint32_t *)image, w, h, 200, 200, 200, 1000, 500, 100, 250, ptr);

	for (int i=0; i<w*h; i++){
		data_p[3*i + 2] = image[4*i + 0];
		data_p[3*i + 1] = image[4*i + 1];
		data_p[3*i + 0] = image[4*i + 2];
	}

	//Specify an Absolute Path for Storing Output Image
	cv::imwrite("/home/yf4418/eie-project19/img/red.jpg",src_hls);

	free(image);

	return 0;

}
