#include <QtGui/QApplication>
#include "skeletonvisualization.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    skeletonVisualization w;
    w.show();
    
    return a.exec();
}
