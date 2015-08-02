#include <iostream>
#include <cstdio>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <fstream>
#include <math.h>
#include <algorithm>

using namespace cv;
using namespace std;

struct verticalLineSegment
{
	int count;
	int ymin;
	int ymax;

	verticalLineSegment()
	{
		count = 0;
		ymax = 0;
		ymin = 0;
	}
};

struct LineSegmentsBlock
{
	int startx;
	int endx;
	int ymin;
	int ymax;

	LineSegmentsBlock()
	{
		startx = 0;
		endx = 0;
		ymin = 0;
		ymax = 0;
	}
};

class BookSegmenter
{
private:
	int imagewidth, imageheight;
	
	Mat CannyThreshold(Mat src_gray);
	vector<Vec4i> extractLines(Mat input);
	bool isHorizental(Vec4i line, float maxrotation);
	vector<Vec4i> filterHorizentalLines(vector<Vec4i> lines,float maxrotation);
	Vec4i segmentLinesSequence(vector<Vec4i> lines, int max_spacing,int min_segements);
	Mat drawLines(Mat input, vector<Vec4i> lines);
	void getMaxLinesCoordinates(vector<Vec4i> lines, int& xmin, int& ymin,int& xmax, int& ymax);
	Vector<LineSegmentsBlock*> findLineBlocks(verticalLineSegment* segments,int length, int max_spacing, int min_segments);


	
public:
	void runSegmenter(Mat input, Point& p1, Point& p2);
	
	void resizeImg(const Mat& inimg,Mat& outimg, int w, int h) {
		Size s(w, h);
		resize(inimg, outimg, s);
	}
};
