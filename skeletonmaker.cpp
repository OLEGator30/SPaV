#include "SkeletonMaker.h"

SkeletonMaker::SkeletonMaker(void)
{
	ready = true;
	
#ifdef _UNICODE
	DllHandle = LoadLibraryW( L"BSTransLib.dll" );
#else	
	DllHandle = LoadLibraryA("BSTransLib.dll");
#endif

	if (DllHandle == NULL) {
		// ошибка загрузки DLL
		ready = false;
		return;
	}

	BSTransform = (BSTransformFunctionType)GetProcAddress(DllHandle, "BSTransform");
   
	if (BSTransform == NULL) {
		// ошибка загрузки BSTransform
		ready = false;
		return;
	}

	BSRemove = (BSRemoveFunctionType)GetProcAddress(DllHandle, "BSRemove");
   
	if (BSRemove == NULL) {
		// ошибка загрузки BSRemove
		ready = false;
		return;
	}
}

SkeletonMaker::~SkeletonMaker(void)
{
	if (DllHandle != NULL) {
		FreeLibrary(DllHandle);
	}
}

SkeletonMaker::Skeleton SkeletonMaker::createSkeleton(SourceImage* image, int pruningParameter, int areaIgnoreParameter)
{
	clock_t time0 = clock();

	Skeleton skeleton;

	// если удалось загрузить dll
	if (ready) {
		
		// преобразование изображения
		DllSourceImage dllImage;
		dllImage.height = image->height;
		dllImage.width = image->width;
		dllImage.pixelData = (int*)(image->pixels); // просто копируем указатель
		
		// вызов функции создания скелета внешней dll
		int* dllResultPtr;
		int dllResultSize = BSTransform(&dllImage, (UINT16)pruningParameter, (UINT16)areaIgnoreParameter, &dllResultPtr);
		DllBoundarySkeleton* dllSkeleton = (DllBoundarySkeleton*)dllResultPtr;

		// преобразование контуров
		DllContourOutline* dllContours = (DllContourOutline*)(dllSkeleton->contours);
		skeleton.contours.resize(dllSkeleton->contoursCount);
		
		for (int i = 0; i < dllSkeleton->contoursCount; i++) {
			
			skeleton.contours[i].isInternal = dllContours[i].isInternal;
			skeleton.contours[i].corners.resize(dllContours[i].cornerCount);

			for (int j = 0; j < dllContours[i].cornerCount; j++) {
				skeleton.contours[i].corners[j] = ContourCorner(((DllContourCorner*)(dllContours[i].cornerList))[j].x,
					((DllContourCorner*)(dllContours[i].cornerList))[j].y);
			}
		}

		// преобразование компонент
		DllComponent* dllComponents = (DllComponent*)(dllSkeleton->components);
		skeleton.components.resize(dllSkeleton->componentsCount);
		for (int i = 0; i < dllSkeleton->componentsCount; i++) {

			skeleton.components[i].borderIndex = dllComponents[i].borderIndex;
			
			skeleton.components[i].bones.resize(dllComponents[i].bonesCount);
			for (unsigned int j = 0; j < dllComponents[i].bonesCount; j++) {
				skeleton.components[i].bones[j].i = ((DllSkeletonBone*)(dllComponents[i].bones))[j].Destin;
				skeleton.components[i].bones[j].j = ((DllSkeletonBone*)(dllComponents[i].bones))[j].Origin;
				
				if (((DllSkeletonBone*)(dllComponents[i].bones))[j].Control >= 0) {
					skeleton.components[i].bones[j].controlX = ((DllContourCorner*)(dllComponents[i].controls))
						[((DllSkeletonBone*)(dllComponents[i].bones))[j].Control].x;
					skeleton.components[i].bones[j].controlY = ((DllContourCorner*)(dllComponents[i].controls))
						[((DllSkeletonBone*)(dllComponents[i].bones))[j].Control].y;
				}
				else {
					skeleton.components[i].bones[j].controlX = -1;
					skeleton.components[i].bones[j].controlY = -1;
				}
			}

			skeleton.components[i].nodes.resize(dllComponents[i].nodesCount);
			for (unsigned int j = 0; j < dllComponents[i].nodesCount; j++) {
				skeleton.components[i].nodes[j].x = ((DllSkeletonNode*)(dllComponents[i].nodes))[j].X;
				skeleton.components[i].nodes[j].y = ((DllSkeletonNode*)(dllComponents[i].nodes))[j].Y;
				skeleton.components[i].nodes[j].r = ((DllSkeletonNode*)(dllComponents[i].nodes))[j].R;
				
				for (int k = 0; k < 3; k++) {
					skeleton.components[i].nodes[j].sites[k].contourCornerIndex = 
						((DllSkeletonNode*)(dllComponents[i].nodes))[j].sites[k].Corner;
					skeleton.components[i].nodes[j].sites[k].contourIndex = 
						((DllSkeletonNode*)(dllComponents[i].nodes))[j].sites[k].Outline;
					skeleton.components[i].nodes[j].sites[k].type = 
						((DllSkeletonNode*)(dllComponents[i].nodes))[j].sites[k].Kind;
				}
			}

			skeleton.components[i].holes.resize(dllComponents[i].holesCount);
			for (int j = 0; j < dllComponents[i].holesCount; j++) {
				skeleton.components[i].holes[j] = ((UINT16*)(dllComponents[i].holes))[j];
			}
		}

		skeleton.traceTime = dllSkeleton->traceTime;
		skeleton.incidenceTreeTime = dllSkeleton->incidenceTreeTime;
		skeleton.skeletizationTime = dllSkeleton->skeletizationTime;
		skeleton.prunningTime = dllSkeleton->prunningTime;
		skeleton.totalTime = dllSkeleton->totalTime;

		BSRemove(&dllResultPtr);
	}

	clock_t time1 = clock();

	skeleton.cppTime = 1000 * double(time1 - time0) / CLOCKS_PER_SEC;

	return skeleton;
}
