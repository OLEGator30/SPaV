#ifndef SKELETONVISUALIZATION_H
#define SKELETONVISUALIZATION_H

#include <QtGui/QMainWindow>
#include "ui_skeletonvisualization.h"

#include <QFileDialog>
#include <string>
using std::string;

#include "SkeletonMaker.h"

class skeletonVisualization : public QMainWindow
{
	Q_OBJECT

public:
    skeletonVisualization(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~skeletonVisualization();

private:
	Ui::SkeletonVisualizationClass ui;
	QString imagepath;
	QImage image;
    QGraphicsScene *scene;
	SkeletonMaker::Skeleton skeleton;
    SkeletonMaker skeletonMaker;

    // visualization flags
    bool drawBones;     // draw by default
    bool drawCircles;   // doesn't draw by default
    bool drawContours;  // draw by default
    bool drawImage;     // draw by default
    bool skeletonView;  // true - internal (by default), false - external
    bool ready;         // WTF? (true by default)

    float scale;        // scale of image (1 by default)

    byte toGrayscale(QRgb pix);
    void updateImage();

private slots:
	void openImageButtonClicked();
	void checkBoxesChanged(int);
	void updateSkeleton();
	void saveImage();
    void breaksConnector();
    void exitMethod();
    void scaleOn();
    void scaleOff();
    void scaleOrig();
    void internal();
    void external();
};

#endif // SKELETONVISUALIZATION_H
