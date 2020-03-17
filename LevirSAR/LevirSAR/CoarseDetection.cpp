#include "CoarseDetection.h"

CoarseDetection::CoarseDetection()
{
    // ��˹�˲�ģ���С
    this->kernel_size1 = 5;
    this->kernel_size2 = 25;
	//��̬ѧ�˲�ģ���С
    this->close_size = 25;
    this->open_size = 3;
	// ��ѡ��ɸѡ��ز���
    this->min_width = 2;
	this->min_height = 2;
	this->min_ratio = (float)0.1;
    this->max_ratio = 10;
	this->min_area = 20;
}

CoarseDetection::~CoarseDetection()
{
}

void CoarseDetection::run(Mat &img, vector<Rect> &targets)
{
	connect_search(get_candi_map(img), targets);
}

// �õ���ѡ����ͼ
Mat CoarseDetection::get_candi_map(Mat &img)
{
#ifdef DEBUG
	clock_t starttime = clock();
#endif // DEBUG
	
	// ��ͬ�˸�˹�˲�
    Mat gauss_map1, gauss_map2;
    GaussianBlur(img, gauss_map1, Size(kernel_size1, kernel_size1), 0);
	GaussianBlur(img, gauss_map2, Size(kernel_size2, kernel_size2), 0);

#ifdef DEBUG
	/*imwrite("./gauss_map1.jpg", gauss_map1);
	imwrite("./gauss_map2.jpg", gauss_map2);*/

	clock_t endtime = clock();
	cout << "gaussblur:" << (endtime - starttime) / 1000. << " s" << endl;

	starttime = clock();
#endif // DEBUG

	// ��һ���õ�����Ŀ��ͱ����ĸ���
	/*Mat gauss_prob1, gauss_prob2;
	gauss_map1.convertTo(gauss_prob1, CV_32F);
	gauss_map2.convertTo(gauss_prob2, CV_32F);*/
	normalize(gauss_map1, gauss_map1, 128, 0, NORM_MINMAX);
	normalize(gauss_map2, gauss_map2, 128, 0, NORM_MINMAX);

	// ��ֵ�ָ��ж�����Ŀ�껹�Ǳ���
	threshold(gauss_map1 + gauss_map2, gauss_map1, 128, 255, THRESH_BINARY);
	/*gauss_map1.convertTo(gauss_map1, CV_8U);*/

#ifdef DEBUG
	endtime = clock();
	cout << "threshold:" << (endtime - starttime) / 1000. << " s" << endl;

	//imwrite("./threshold.jpg", gauss_map1);

	starttime = clock();
#endif // DEBUG

	// ���������׶�
	Mat kernel1 = getStructuringElement(MORPH_RECT, Size(close_size, close_size));
	morphologyEx(gauss_map1, gauss_map1, MORPH_CLOSE, kernel1);
    /*dilate(gauss_map1, gauss_map1, kernel1);
	erode(gauss_map1, gauss_map1, kernel1);*/

#ifdef DEBUG
	//imwrite("./close.jpg", gauss_map1);
#endif // DEBUG

    // �������������
	Mat kernel2 = getStructuringElement(MORPH_RECT, Size(open_size, open_size));
	morphologyEx(gauss_map1, gauss_map1, MORPH_OPEN, kernel2);
	/*erode(gauss_map1, gauss_map1, kernel2);
	dilate(gauss_map1, gauss_map1, kernel2);*/

#ifdef DEBUG
	//imwrite("./open.jpg", gauss_map1);

	endtime = clock();
	cout << "filter:" << (endtime - starttime) / 1000. << " s" << endl;
#endif // DEBUG

    return gauss_map1;
}

// ��ͨ�����
void CoarseDetection::connect_search(Mat &candiMap, vector<Rect> &targets)
{
#ifdef DEBUG
	clock_t starttime = clock();
#endif // DEBUG

	// Ѱ����ͨ����
	vector<vector<Point> > contours;
    findContours(candiMap, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	Rect boundingbox;
	int box_width;
	int box_height;
	float aspect_ratio;
	for (int i(0); i<contours.size(); i++)
    {
		// �������Χ��
		boundingbox = boundingRect(contours[i]);
		box_width = boundingbox.width;
		box_height = boundingbox.height;
		aspect_ratio = (float)box_height / box_width;

		// ��ѡ��ɸѡ
		if (box_width < min_width || box_height < min_height || box_width*box_height < min_area
			|| aspect_ratio < min_ratio || aspect_ratio > max_ratio)
		{
			continue;
		}

        targets.push_back(boundingbox);
    }
    contours.clear();
#ifdef DEBUG
	clock_t endtime = clock();
	cout << "connect search:" << (endtime - starttime) / 1000. << " s" << endl;
#endif // DEBUG
}
