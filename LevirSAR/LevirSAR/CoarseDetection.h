#ifndef COARSEDETECTION_H
#define COARSEDETECTION_H

#include "LevirSAR.h"

class CoarseDetection
{
private:
	// �������ڴ�С
	int target_size;
	int guard_size;
	int background_size;
	//��̬ѧ�˲�ģ���С
	int close_size;
	int open_size;
	// ��ѡ��ɸѡ��ز���
	int min_width;
	int min_height;
	int max_width;
	int max_height;
	float min_ratio;
	float max_ratio;
	int min_area;
	int max_area;
	// �˲�ģ��
	Mat background_kernel;
	// ��˹ģ�壨FPGA��
	double* gaussian_template;
	// ����ģ�壨FPGA��
	double* squarehole_template;

	// �����˲�ģ��
	Mat get_background_kernel();
	// �õ���ѡ����ͼ
	Mat get_candi_map(Mat& img);
	// ���ɸ�˹ģ�壨FPGA��
	void get_gaussian_template_fpga(double* gaussian_template, int kernel_size, double sigma = 0);
	// ���ɻ���ģ�壨FPGA��
	void get_squarehole_template_fpga(double* squarehole_template, int background_size, int guard_size);
	// �˲���FPGA��
	void filter_fpga(uchar* img_src, uchar* img_dst, int img_row, int img_col, int kernel_size, double* filter_template);
	// �õ���ѡ����ͼ��FPGA��
	Mat get_candi_map_fpga(Mat& img);
	// ��ͨ�����
	void connect_search(Mat& candiMap, vector<Rect>& targets);

public:
	CoarseDetection();
	~CoarseDetection();

	void run(Mat& img, vector<Rect>& targets);
};

#endif
