#include <iostream>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include "../headers/BookSegmenter.h";

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	BookSegmenter bs;
	if(argc<2) {
		cout<<"Invalid number of arguments"<<endl;
		return 0;
	}
	
	//Mat in_img = imread("/home/divoo/Public/Rmagine_pages/bad/IMAG0625.jpg",1);
	Mat in_img = imread(argv[1]);
	
	//resize image
	//bs.resizeImg(in_img, in_img, in_img.size().width / 4,in_img.size().height / 4);

	Point p1;
	Point p2;
	bs.runSegmenter(in_img,p1,p2);
	cout<<"final boundry "<<p1.x<<","<<p1.y<<" "<<p2.x<<","<<p2.y<<endl;

	waitKey(0);
	return 0;
}
