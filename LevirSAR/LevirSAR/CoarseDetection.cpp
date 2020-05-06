#include "CoarseDetection.h"

CoarseDetection::CoarseDetection()
{
	// �������ڴ�С
	target_size = 3;
	guard_size = 25;
	background_size = 50;
	//��̬ѧ�˲�ģ���С
	close_size = 25;
	open_size = 3;
	// ��ѡ��ɸѡ��ز���
	min_width = 2;
	min_height = 2;
	max_width = 400;
	max_height = 400;
	min_ratio = 0.1;
	max_ratio = 10;
	min_area = 20;
	max_area = 28000;

	// �����˲�ģ��
	background_kernel = get_background_kernel();
	// ���ɸ�˹ģ�壨FPGA��
	// gaussian_template = new double[target_size * target_size];
	// get_gaussian_template_fpga(gaussian_template, target_size);
	// ���ɻ���ģ�壨FPGA��
	// squarehole_template = new double[background_size * background_size];
	// get_squarehole_template_fpga(squarehole_template, background_size, guard_size);
}

CoarseDetection::~CoarseDetection()
{
	// delete[] gaussian_template;
	// delete[] squarehole_template;
}

void CoarseDetection::run(Mat& img, vector<Rect>& targets)
{
	connect_search(get_candi_map(img), targets);
}

// �����˲�ģ��
Mat CoarseDetection::get_background_kernel()
{
	int start = (background_size - guard_size) / 2;
	int end = start + guard_size;
	Mat background_kernel(background_size, background_size, CV_32FC1, Scalar(1.0));
	background_kernel.colRange(start, end).rowRange(start, end).setTo(0.0);
	background_kernel = background_kernel / (background_size * background_size - guard_size * guard_size);
	return background_kernel;
}

// �õ���ѡ����ͼ
Mat CoarseDetection::get_candi_map(Mat& img)
{
	// ��ͬ�˸�˹���ֵ�˲�
	Mat target_map, background_map;
	GaussianBlur(img, target_map, Size(target_size, target_size), 0);
	resize(img, background_map, Size(img.cols / 4, img.rows / 4));
	filter2D(background_map, background_map, -1, background_kernel);
	resize(background_map, background_map, Size(img.cols, img.rows));

	// ��ֵ�ָ��ж�����Ŀ�껹�Ǳ���
	threshold(target_map - background_map, target_map, 50, 255, THRESH_BINARY);

	// ���������׶�
	Mat kernel1 = getStructuringElement(MORPH_RECT, Size(close_size, close_size));
	morphologyEx(target_map, target_map, MORPH_CLOSE, kernel1);

	// �������������
	Mat kernel2 = getStructuringElement(MORPH_RECT, Size(open_size, open_size));
	morphologyEx(target_map, target_map, MORPH_OPEN, kernel2);

	return target_map;
}

// ���ɸ�˹ģ�壨FPGA��
void CoarseDetection::get_gaussian_template_fpga(double* gaussian_template, int kernel_size, double sigma)
{
	static const double pi = 3.1415926535;
	if (sigma == 0)
	{
		sigma = ((kernel_size - 1) * 0.5 - 1) * 0.3 + 0.8;  // �˴����õ���opencv����API��sigma���㷽ʽ
	}

	if (kernel_size % 2 != 1)
	{
		printf("kernel_size ӦΪ����\n");
	}

	int center = kernel_size / 2;
	double x2, y2;
	double sum = 0.0;

	for (int i = 0; i < kernel_size; i++)
	{
		// x2 = (i - center) * (i - center);
		x2 = pow(double(i - center), 2);
		for (int j = 0; j < kernel_size; j++)
		{
			y2 = pow(double(j - center), 2);
			double tmp = exp(-(x2 + y2) / (2 * sigma * sigma));
			tmp /= 2 * pi * sigma;
			sum += tmp;
			gaussian_template[i * kernel_size + j] = tmp;
		}
	}

	// ��һ��
	for (int i = 0; i < kernel_size; i++)
	{
		for (int j = 0; j < kernel_size; j++)
		{
			gaussian_template[i * kernel_size + j] /= sum;
		}
	}
}

// ���ɻ���ģ�壨FPGA��
void CoarseDetection::get_squarehole_template_fpga(double* squarehole_template, int background_size, int guard_size)
{
	if (background_size <= guard_size + 2)  // Ҫ�󱣻����ڳߴ�С�ڱ������ڳߴ�
	{
		printf("Ҫ�� �������ڳߴ�+2 С�� �������ڳߴ�\n");
	}
	int idx_min = (background_size - guard_size) / 2;
	int idx_max = idx_min + guard_size - 1;
	double rate = 1.0 / (background_size * background_size - guard_size * guard_size);
	for (int i = 0; i < background_size; i++)
	{
		for (int j = 0; j < background_size; j++)
		{
			if (i < idx_min || i > idx_max || j < idx_min || j > idx_max)
			{
				squarehole_template[i * background_size + j] = rate;
			}
			else
			{
				squarehole_template[i * background_size + j] = 0;
			}
			//printf("%lf\n", squarehole_template[i * background_size + j]);
		}
	}
}

// �˲���FPGA��
void CoarseDetection::filter_fpga(uchar* img_src, uchar* img_dst, int img_row, int img_col, int kernel_size, double* filter_template)
{
	for (int i = 0; i < img_row; i++)  // ����ͼ�����ر������˴��ɽ���FPGA���м������
	{
		for (int j = 0; j < img_col; j++)
		{
			double tmp_value = 0;
			double tmp = 0;
			for (int m = 0; m < kernel_size; m++)  // ����ģ���ڼ���
			{
				for (int n = 0; n < kernel_size; n++)
				{
					int x_coor = i - kernel_size / 2 + m;
					int y_coor = j - kernel_size / 2 + n;
					if (x_coor < 0 || x_coor >= img_row || y_coor < 0 || y_coor >= img_col)
					{
						tmp = 0;
					}
					else
					{
						tmp = img_src[x_coor * img_col + y_coor] * filter_template[m * kernel_size + n];
					}
					tmp_value += tmp;
				}
			}
			img_dst[i * img_col + j] = tmp_value;
		}
	}
}

// �õ���ѡ����ͼ��FPGA��
Mat CoarseDetection::get_candi_map_fpga(Mat& img)
{
#ifdef TIME_DEBUG
	clock_t starttime = clock();
#endif // TIME_DEBUG

	// ��ͬ�˸�˹���ֵ�˲�
	Mat target_map, background_map;
	resize(img, background_map, Size(img.cols / 4, img.rows / 4));

	// �ò��ֿ�����FPGA����
	uchar* target_src = img.ptr<uchar>(0);
	int target_row = img.rows;
	int target_col = img.cols;
	uchar* target_dst = new uchar[target_row * target_col];
	filter_fpga(target_src, target_dst, target_row, target_col, target_size, gaussian_template);

	uchar* background_src = background_map.ptr<uchar>(0);
	int background_row = background_map.rows;
	int background_col = background_map.cols;
	uchar* background_dst = new uchar[background_row * background_col];
	filter_fpga(background_src, background_dst, background_row, background_col, background_size, squarehole_template);

	target_map = Mat(target_row, target_col, CV_8UC1, target_dst).clone();
	background_map = Mat(background_row, background_col, CV_8UC1, background_dst).clone();

	delete[] target_dst;
	delete[] background_dst;
	resize(background_map, background_map, Size(img.cols, img.rows));

#ifdef TIME_DEBUG
	clock_t endtime = clock();
	cout << "blur:" << (endtime - starttime) / 1000. << " s" << endl;

	starttime = clock();
#endif // TIME_DEBUG

	// ��ֵ�ָ��ж�����Ŀ�껹�Ǳ���
	threshold(target_map - background_map, target_map, 50, 255, THRESH_BINARY);

#ifdef TIME_DEBUG
	endtime = clock();
	cout << "threshold:" << (endtime - starttime) / 1000. << " s" << endl;

	starttime = clock();
#endif // TIME_DEBUG

	// ���������׶�
	Mat kernel1 = getStructuringElement(MORPH_RECT, Size(close_size, close_size));
	morphologyEx(target_map, target_map, MORPH_CLOSE, kernel1);

	// �������������
	Mat kernel2 = getStructuringElement(MORPH_RECT, Size(open_size, open_size));
	morphologyEx(target_map, target_map, MORPH_OPEN, kernel2);

#ifdef TIME_DEBUG
	endtime = clock();
	cout << "filter:" << (endtime - starttime) / 1000. << " s" << endl;
#endif // TIME_DEBUG

	return target_map;
}

// ��ͨ�����
void CoarseDetection::connect_search(Mat& candiMap, vector<Rect>& targets)
{
	// Ѱ����ͨ����
	vector<vector<Point> > contours;
	findContours(candiMap, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	Rect boundingbox;
	int box_width;
	int box_height;
	float aspect_ratio;
	int area;
	for (int i(0); i < contours.size(); i++)
	{
		// �������Χ��
		boundingbox = boundingRect(contours[i]);
		box_width = boundingbox.width;
		box_height = boundingbox.height;
		aspect_ratio = (float)box_height / box_width;
		area = box_width * box_height;

		// ��ѡ��ɸѡ
		if (box_width < min_width || box_height < min_height || area < min_area
			|| box_width > max_width || box_height > max_height || area < min_area
			|| aspect_ratio < min_ratio || aspect_ratio > max_ratio)
		{
			continue;
		}

		targets.push_back(boundingbox);
	}
	contours.clear();
}
