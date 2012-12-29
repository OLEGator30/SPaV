/*
	Обёртка C++ для библиотеки скелетизации Л.М. Местецкого (BSTransLib.dll)
*/

#pragma once

#include "windows.h" // для подключения внешней библиотеки dll под Windows
#include <vector>
using std::vector;
#include <ctime> // замер времени работы

class SkeletonMaker
{
	/*
		Описание структур данных исходной dll. Для совместимости с исходной dll
		используется ручное выравнивание полей в структурах
		с использованием пустых полей blank# нужного размера.
	*/

	// исходное изображение
	__declspec(align(1)) struct DllSourceImage
	{
		UINT16 height; // количество строк
		UINT16 width; // количество столбцов
		int* pixelData; // byte[height * width]: 0 - объект, 255 - фон
	};
	
	// гранично-скелетное описание фигуры
	__declspec(align(1)) struct DllBoundarySkeleton
	{
		UINT16 contoursCount; // количество граничных контуров фигуры
		UINT16 blank1;
		int* contours; // список граничных контуров фигуры - массив из DllContourOutline
		UINT16 componentsCount; // количество связных компонент фигуры
		UINT16 blank2;
		int* components; // список связных компонент фигуры - массив из DllComponent
		UINT32 traceTime;
		UINT32 incidenceTreeTime;
		UINT32 skeletizationTime;
		UINT32 prunningTime;
		UINT32 totalTime;
	};

	// граничный контур
	__declspec(align(1)) struct DllContourOutline
	{
		byte isInternal; // внешний - 0 или внутренний - 1 контур
		byte blank1;
		UINT16 cornerCount; // число вершин в многоугольнике
		int* cornerList; // список вершин многоугольника - массив из DllContourCorner
	};

	// вершина многоугольника - граничного контура
	__declspec(align(1)) struct DllContourCorner
	{
		float x, y; // координаты вершины
	};

	// компонента связности фигуры
	__declspec(align(1)) struct DllComponent
	{
		UINT16 borderIndex; // номер внешнего контура границы
		UINT16 holesCount; // число внутренних граничных контуров (дыр)
		int* holes; // список внутренних контуров (дыр) - массив из UINT16
		UINT32 nodesCount; // число узлов в скелете компоненты
		int* nodes; // список узлов в скелете компоненты - массив из DllSkeletonNode
		UINT32 bonesCount; // число костей в скелете компоненты
		int* bones; // список костей в скелете компоненты - массив из DllSkeletonBone
		UINT32 controlsCount; // число параболических костей в скелете компоненты
		int* controls; // список контрольных точек параболических костей - массив из DllContourCorner
	};

	// инцидентный сайт
	__declspec(align(1)) struct DllIncidentSite
	{
		UINT16 Outline; // номер контура фигуры
		UINT16 Corner; // номер вершины в контуре
		byte Kind; // 0 - сайт-точка, 1 - сайт-сегмент
		byte blank1;
	};

	// узел скелета
	__declspec(align(1)) struct DllSkeletonNode
	{
		float X, Y, R;
		DllIncidentSite sites[3];
		byte blank1;
		byte blank2;
	};

	// кость (ребро) скелета
	__declspec(align(1)) struct DllSkeletonBone
	{
		int Origin; // начальный узел кости
		int Destin; // конечный узел кости
		int Control; // контрольный узел кости (для параболического ребра)
	};

	typedef int (__cdecl* BSTransformFunctionType)(DllSourceImage*, UINT16, UINT16, int**);
	typedef int (__cdecl* BSRemoveFunctionType)(int**);
	
	bool ready;
	HINSTANCE DllHandle;
	BSTransformFunctionType BSTransform;
	BSRemoveFunctionType BSRemove;

public:
	/*
		Описание возвращаемого скелета
	*/
	
	// инцидентный сайт
	struct incidentSite
	{
		int contourIndex; // номер контура фигуры
		int contourCornerIndex; // номер вершины в контуре
		byte type; // 0 - сайт-точка, 1 - сайт-сегмент
		// для сайта-сегмента парная вершина будет следующей вершиной в контуре
	};

	// узел скелета
	struct SkeletonNode
	{
		float x, y; // координаты узла
		float r; // радиус максимального вписанного круга в точке (x, y)
		incidentSite sites[3]; // инцидентные максимальному кругу сайты

		SkeletonNode() {}
		SkeletonNode(float x_, float y_, float r_)
		{
			x = x_; y = y_; r = r_;
		}
	};

	// ребро скелета
	struct SkeletonBone
	{
		int i, j; // пара индексов соединяемых узлов
		float controlX, controlY; // координаты контрольной точки (для параболического ребра)

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

	// вершина контура
	struct ContourCorner
	{
		float x, y; // координаты вершины
		
		ContourCorner() {}
		ContourCorner(float x_, float y_)
		{
			x = x_; y = y_;
		}
	};

	// контур
	struct ContourOutline
	{
		byte isInternal; // внешний - 0 или внутренний - 1 контур
		vector<ContourCorner> corners; // список вершин контура
	};

	// связная компонента
	struct Component
	{
		int borderIndex; // индекс внешнего контура
		vector<int> holes; // индексы внутренних граничных контуров (дыр)
        vector<SkeletonNode> nodes;
		vector<SkeletonBone> bones;
	};

	struct Skeleton
	{
		vector<ContourOutline> contours; // список граничных контуров фигуры
		vector<Component> components; // количество связных компонент фигуры
		int traceTime;
		int incidenceTreeTime;
		int skeletizationTime;
		int prunningTime;
		int totalTime;
		int cppTime; // время построения скелета + время, потраченное на преобразование структур, в миллисекундах
	};

	// исходное изображение
	struct SourceImage
	{
		int height; // количество строк
		int width; // количество столбцов
		byte* pixels; // byte[height * width]: 0 - объект, 255 - фон
	};
	
	SkeletonMaker(void);
	~SkeletonMaker(void);

	// основная процедура для построения скелета
	Skeleton createSkeleton(SourceImage* image, int pruningParameter, int areaIgnoreParameter);
};
