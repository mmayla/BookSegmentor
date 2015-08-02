#include "../headers/BookSegmenter.h";

void BookSegmenter::runSegmenter(Mat input, Point& p1, Point& p2)
{
	//get image width and height
	imagewidth = input.size().width;
	imageheight = input.size().height;
	cout << "image dim(hxw): " << imageheight << "x" << imagewidth << endl;

	//convert to gray scale image
	Mat gray_input;
	cvtColor(input, gray_input, CV_BGR2GRAY);

	//Gaussian blur
	GaussianBlur(gray_input, gray_input, Size(5, 5), 0);

	//canny edge detection
	Mat edges = CannyThreshold(gray_input);

	//extract lines
	vector<Vec4i> lines = extractLines(edges);

	//filter lines
	lines = filterHorizentalLines(lines, 0.1);

	//segment lines
	Vec4i segment_boundry = segmentLinesSequence(lines, 0, 10);

	//draw lines
	Mat canvas = drawLines(gray_input, lines);

	//draw boundry
	p1.x = segment_boundry[0];
	p1.y = segment_boundry[1];
	p2.x = segment_boundry[2];
	p2.y = segment_boundry[3];
	
	rectangle(canvas, p1, p2, Scalar(255, 0, 0), 3, CV_AA);
	imshow("canvas", canvas);
}

Mat BookSegmenter::CannyThreshold(Mat src_gray){
	int lowThreshold = 30;
	int ratio = 3;
	int kernel_size = 3;

	/// Reduce noise with a kernel 3x3
	Mat detected_edges;
	blur(src_gray, detected_edges, Size(3, 3));

	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * ratio,
			kernel_size);

	/// Using Canny's output as a mask, we display our result
	//Mat dst = Scalar::all(0);

	//src_gray.copyTo( dst, detected_edges);
	//imshow("Canny", detected_edges);

	return detected_edges;
}

vector<Vec4i> BookSegmenter::extractLines(Mat input){
	vector<Vec4i> lines;
	HoughLinesP(input, lines, 1, CV_PI / 180, 50, 50, 10);

	return lines;
}

bool BookSegmenter::isHorizental(Vec4i line, float maxrotation = 0.1){
	int x1 = line[0];
	int y1 = line[1];
	int x2 = line[2];
	int y2 = line[3];
	float m = (float) abs(y2 - y1) / (float) abs(x2 - x1);
	cout << ">" << m << " x1:" << x1 << " y1:" << y1 << " x2:" << x2 << " y2:"
			<< y2 << endl;
	if (m >= 0 && m < maxrotation)
		return true;
	else
		return false;
}

vector<Vec4i> BookSegmenter::filterHorizentalLines(vector<Vec4i> lines,float maxrotation = 0.1){
	vector<Vec4i> filtred;

	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];

		if (isHorizental(l))
		{
			filtred.push_back(l);
		}
	}

	cout << "filter ends" << endl;

	return filtred;
}

Vector<LineSegmentsBlock*> BookSegmenter::findLineBlocks(verticalLineSegment* segments,int length, int max_spacing, int min_segments){
	Vector<LineSegmentsBlock*> blocks;

	bool insequence = false;
	LineSegmentsBlock* block = NULL;
	vector<int> min_y;
	vector<int> max_y;
	int lastspacing = 0;
	for (int i = 0; i < length; i++)
	{
		if (segments[i].count >= min_segments)
		{
			if (!insequence)
			{
				block = new LineSegmentsBlock();
				block->startx = i;
				block->endx = i;
				min_y.clear();
				max_y.clear();
				insequence = true;
				lastspacing = 0;
			}

			block->endx = i;
			min_y.push_back(segments[i].ymin);
			max_y.push_back(segments[i].ymax);
		}
		else
		{
			//TODO: implement spacing property
			lastspacing++;
			insequence = false;
			if (block != NULL)
			{
				if ((block->endx - block->startx) > min_segments)
				{
					sort(max_y.begin(), max_y.end());
					sort(min_y.begin(), min_y.end());
					block->ymax = max_y[max_y.size()/2];
					block->ymin = min_y[min_y.size()/2];

					blocks.push_back(block);
				}
			}
			block = NULL;
		}
	}

	return blocks;
}

Vec4i BookSegmenter::segmentLinesSequence(vector<Vec4i> lines, int max_spacing,int min_segements){
	//get max and min line values
	int xmin, xmax, ymin, ymax;
	getMaxLinesCoordinates(lines, xmin, ymin, xmax, ymax);
	cout << "x: " << xmin << "," << xmax << " y: " << ymin << "," << ymax << " "
			<< endl;

	//initial initialization
	verticalLineSegment* segments = new verticalLineSegment[xmax - xmin];
	int seglength = (xmax - xmin);
	for (int i = 0; i < seglength; i++)
	{
		segments[i].count = 0;
		segments[i].ymax = ymin;
		segments[i].ymin = ymin;
	}

	//fill the counting map
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		int x1 = min(l[0], l[2]);
		int y1 = min(l[1], l[3]);
		int x2 = max(l[0], l[2]);
		int y2 = max(l[1], l[3]);

		for (int k = x1 - xmin; k < x2 - xmin; k++)
		{
			segments[k].count++;
			segments[k].ymax = max(segments[k].ymax, y2);
			segments[k].ymin = min(segments[k].ymin, y1);
		}
	}

//	cout<<"count: \n";
//	for(int i=0;i<(xmax-xmin);i++){
//		cout<<segments[i].count<<" ";
//	}cout<<endl;

	//get blocks
	Vector<LineSegmentsBlock*> blocks = findLineBlocks(segments, seglength,
			max_spacing, min_segements);

	//get largest block
	LineSegmentsBlock* max = NULL;
	for (unsigned int i = 0; i < blocks.size(); i++)
	{
		if (i == 0)
			max = blocks[i];

		int maxlen = max->endx - max->startx;
		int blocklen = blocks[i]->endx - blocks[i]->startx;
		if (blocklen > maxlen)
			max = blocks[i];
	}

	//normalize block coordinates
	max->startx = max->startx + xmin;
	max->endx = max->endx + xmin;

	//draw boundry
	cout << "boundry- x: " << max->startx << "," << max->endx << " y:" << max->ymin
			<< "," << max->ymax << endl;

	cout << "Line segmenetation finished" << endl;
	Vec4i v(max->startx, max->ymin, max->endx, max->ymax);
	return v;
}

Mat BookSegmenter::drawLines(Mat input, vector<Vec4i> lines){
	Mat cdst;
	//convert to RGB domain
	cvtColor(input, cdst, CV_GRAY2BGR);

	cout << "lines: " << lines.size() << endl;
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 1,
		CV_AA);
	}
	//imshow("lines", cdst);
	return cdst;
}

void BookSegmenter::getMaxLinesCoordinates(vector<Vec4i> lines, int& xmin, int& ymin,int& xmax, int& ymax){
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		int x1 = l[0];
		int y1 = l[1];
		int x2 = l[2];
		int y2 = l[3];

		if (i == 0)
		{
			xmin = x1;
			ymin = y1;
			xmax = x1;
			ymax = y1;
		}
		//search for min and max values of x and y
		if (min(x1, x2) < xmin)
			xmin = min(x1, x2);
		if (max(x1, x2) > xmax)
			xmax = max(x1, x2);
		if (min(y1, y2) < ymin)
			ymin = min(y1, y2);
		if (max(y1, y2) > ymax)
			ymax = max(y1, y2);

	}
}
