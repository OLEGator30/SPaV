/*
	������ C++ ��� ���������� ������������ �.�. ���������� (BSTransLib.dll)
*/

#pragma once

#include "windows.h" // ��� ����������� ������� ���������� dll ��� Windows
#include <vector>
using std::vector;
#include <ctime> // ����� ������� ������

class SkeletonMaker
{
	/*
		�������� �������� ������ �������� dll. ��� ������������� � �������� dll
		������������ ������ ������������ ����� � ����������
		� �������������� ������ ����� blank# ������� �������.
	*/

	// �������� �����������
	__declspec(align(1)) struct DllSourceImage
	{
		UINT16 height; // ���������� �����
		UINT16 width; // ���������� ��������
		int* pixelData; // byte[height * width]: 0 - ������, 255 - ���
	};
	
	// ��������-��������� �������� ������
	__declspec(align(1)) struct DllBoundarySkeleton
	{
		UINT16 contoursCount; // ���������� ��������� �������� ������
		UINT16 blank1;
		int* contours; // ������ ��������� �������� ������ - ������ �� DllContourOutline
		UINT16 componentsCount; // ���������� ������� ��������� ������
		UINT16 blank2;
		int* components; // ������ ������� ��������� ������ - ������ �� DllComponent
		UINT32 traceTime;
		UINT32 incidenceTreeTime;
		UINT32 skeletizationTime;
		UINT32 prunningTime;
		UINT32 totalTime;
	};

	// ��������� ������
	__declspec(align(1)) struct DllContourOutline
	{
		byte isInternal; // ������� - 0 ��� ���������� - 1 ������
		byte blank1;
		UINT16 cornerCount; // ����� ������ � ��������������
		int* cornerList; // ������ ������ �������������� - ������ �� DllContourCorner
	};

	// ������� �������������� - ���������� �������
	__declspec(align(1)) struct DllContourCorner
	{
		float x, y; // ���������� �������
	};

	// ���������� ��������� ������
	__declspec(align(1)) struct DllComponent
	{
		UINT16 borderIndex; // ����� �������� ������� �������
		UINT16 holesCount; // ����� ���������� ��������� �������� (���)
		int* holes; // ������ ���������� �������� (���) - ������ �� UINT16
		UINT32 nodesCount; // ����� ����� � ������� ����������
		int* nodes; // ������ ����� � ������� ���������� - ������ �� DllSkeletonNode
		UINT32 bonesCount; // ����� ������ � ������� ����������
		int* bones; // ������ ������ � ������� ���������� - ������ �� DllSkeletonBone
		UINT32 controlsCount; // ����� �������������� ������ � ������� ����������
		int* controls; // ������ ����������� ����� �������������� ������ - ������ �� DllContourCorner
	};

	// ����������� ����
	__declspec(align(1)) struct DllIncidentSite
	{
		UINT16 Outline; // ����� ������� ������
		UINT16 Corner; // ����� ������� � �������
		byte Kind; // 0 - ����-�����, 1 - ����-�������
		byte blank1;
	};

	// ���� �������
	__declspec(align(1)) struct DllSkeletonNode
	{
		float X, Y, R;
		DllIncidentSite sites[3];
		byte blank1;
		byte blank2;
	};

	// ����� (�����) �������
	__declspec(align(1)) struct DllSkeletonBone
	{
		int Origin; // ��������� ���� �����
		int Destin; // �������� ���� �����
		int Control; // ����������� ���� ����� (��� ��������������� �����)
	};

	typedef int (__cdecl* BSTransformFunctionType)(DllSourceImage*, UINT16, UINT16, int**);
	typedef int (__cdecl* BSRemoveFunctionType)(int**);
	
	bool ready;
	HINSTANCE DllHandle;
	BSTransformFunctionType BSTransform;
	BSRemoveFunctionType BSRemove;

public:
	/*
		�������� ������������� �������
	*/
	
	// ����������� ����
	struct incidentSite
	{
		int contourIndex; // ����� ������� ������
		int contourCornerIndex; // ����� ������� � �������
		byte type; // 0 - ����-�����, 1 - ����-�������
		// ��� �����-�������� ������ ������� ����� ��������� �������� � �������
	};

	// ���� �������
	struct SkeletonNode
	{
		float x, y; // ���������� ����
		float r; // ������ ������������� ���������� ����� � ����� (x, y)
		incidentSite sites[3]; // ����������� ������������� ����� �����

		SkeletonNode() {}
		SkeletonNode(float x_, float y_, float r_)
		{
			x = x_; y = y_; r = r_;
		}
	};

	// ����� �������
	struct SkeletonBone
	{
		int i, j; // ���� �������� ����������� �����
		float controlX, controlY; // ���������� ����������� ����� (��� ��������������� �����)

		SkeletonBone() {}
		SkeletonBone(int i_, int j_, float controlX_, float controlY_)
		{
			i = i_; j = j_; controlX = controlX_; controlY = controlY_;
		}
		SkeletonBone(int i_, int j_)
		{
			i = i_; j = j_; controlX = -1; controlY = -1;
		}
	};

	// ������� �������
	struct ContourCorner
	{
		float x, y; // ���������� �������
		
		ContourCorner() {}
		ContourCorner(float x_, float y_)
		{
			x = x_; y = y_;
		}
	};

	// ������
	struct ContourOutline
	{
		byte isInternal; // ������� - 0 ��� ���������� - 1 ������
		vector<ContourCorner> corners; // ������ ������ �������
	};

	// ������� ����������
	struct Component
	{
		int borderIndex; // ������ �������� �������
		vector<int> holes; // ������� ���������� ��������� �������� (���)
        vector<SkeletonNode> nodes;
		vector<SkeletonBone> bones;
	};

	struct Skeleton
	{
		vector<ContourOutline> contours; // ������ ��������� �������� ������
		vector<Component> components; // ���������� ������� ��������� ������
		int traceTime;
		int incidenceTreeTime;
		int skeletizationTime;
		int prunningTime;
		int totalTime;
		int cppTime; // ����� ���������� ������� + �����, ����������� �� �������������� ��������, � �������������
	};

	// �������� �����������
	struct SourceImage
	{
		int height; // ���������� �����
		int width; // ���������� ��������
		byte* pixels; // byte[height * width]: 0 - ������, 255 - ���
	};
	
	SkeletonMaker(void);
	~SkeletonMaker(void);

	// �������� ��������� ��� ���������� �������
	Skeleton createSkeleton(SourceImage* image, int pruningParameter, int areaIgnoreParameter);
};
