#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;

void resizeImg(const Mat& inimg,Mat& outimg, int w, int h) {
	Size s(w, h);
	resize(inimg, outimg, s);
}
