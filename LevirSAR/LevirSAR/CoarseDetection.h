#ifndef COARSEDETECTION_H
#define COARSEDETECTION_H

#include "LevirSAR.h"

class CoarseDetection
{
private:
    // ��˹�˲�ģ���С
	int kernel_size1;
	int kernel_size2;
	//��̬ѧ�˲�ģ���С
    int close_size;
    int open_size;
    // ��ѡ��ɸѡ��ز���
    int min_width;
	int min_height;
    float min_ratio;
    float max_ratio;
	int min_area;

	// �õ���ѡ����ͼ
	Mat get_candi_map(Mat &img);
	// ��ͨ�����
    void connect_search(Mat &candiMap, vector<Rect> &targets);

public:
    CoarseDetection();
    ~CoarseDetection();

    void run(Mat &img, vector<Rect> &targets);
};

#endif
