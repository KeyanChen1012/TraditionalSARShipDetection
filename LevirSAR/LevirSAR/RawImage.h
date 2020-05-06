#ifndef RAWIMAGE_H
#define RAWIMAGE_H

#include "LevirSAR.h"

class RawImage
{
private:
	char *m_filename;
	int m_width;
	int m_height;

	char *m_bufferFile;

public:
    RawImage(char *filename, int width, int height);
    ~RawImage();

	// ��Mat��ʽ����RAW����ͼ��
	Mat returnWholeImg();
	// ����ͼ�������ϵ�����ͳߴ磬��Mat��ʽ����RAWͼ���е�ͼ���
	Mat returnImgPatch(int x, int y, int width, int height);
};

#endif
