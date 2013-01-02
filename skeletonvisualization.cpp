#include "skeletonvisualization.h"
#include <QMessageBox>

skeletonVisualization::skeletonVisualization(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	ui.centralWidget->setContentsMargins(6, 6, 6, 6);
	ui.centralWidget->setLayout(ui.horizontalLayout);

    connect(ui.buttonOpen, SIGNAL(clicked()),
            this, SLOT(openImage()));
    connect(ui.actionOpen, SIGNAL(activated()),
            this, SLOT(openImage()));

    connect(ui.buttonRefresh, SIGNAL(clicked()),
            this, SLOT(updateSkeleton()));
    connect(ui.actionRefresh, SIGNAL(activated()),
            this, SLOT(updateSkeleton()));

    connect(ui.buttonSave, SIGNAL(clicked()),
            this, SLOT(saveImage()));
    connect(ui.actionSave, SIGNAL(activated()),
            this, SLOT(saveImage()));

    connect(ui.buttonConnect, SIGNAL(clicked()),
            this, SLOT(breaksConnector()));
    connect(ui.actionConnect, SIGNAL(activated()),
            this, SLOT(breaksConnector()));

    connect(ui.buttonQuit, SIGNAL(clicked()),
            this, SLOT(exitMethod()));
    connect(ui.actionQuit, SIGNAL(activated()),
            this, SLOT(exitMethod()));

    connect(ui.sliderScale, SIGNAL(valueChanged(int)),
            this, SLOT(setScaleValue(int)));
    connect(ui.checkBoxCircles, SIGNAL(stateChanged(int)),
            this, SLOT(checkBoxesChanged(int)));
    connect(ui.checkBoxBones, SIGNAL(stateChanged(int)),
            this, SLOT(checkBoxesChanged(int)));
    connect(ui.checkBoxContours, SIGNAL(stateChanged(int)),
            this, SLOT(checkBoxesChanged(int)));
    connect(ui.checkBoxImage, SIGNAL(stateChanged(int)),
            this, SLOT(checkBoxesChanged(int)));

    connect(ui.actionOn, SIGNAL(activated()), this, SLOT(scaleOn()));
    connect(ui.actionOff, SIGNAL(activated()), this, SLOT(scaleOff()));
    connect(ui.actionOriginal, SIGNAL(activated()), this, SLOT(scaleOrig()));
    connect(ui.actionInternal, SIGNAL(activated()), this, SLOT(internal()));
    connect(ui.actionExternal, SIGNAL(activated()), this, SLOT(external()));

    scene = 0;
    drawCircles = ready = 0;
    drawBones = drawImage = drawContours = skeletonView = 1;
    scale = 1.0;
}

skeletonVisualization::~skeletonVisualization()
{
    if (scene)
        delete scene;
}

byte skeletonVisualization::toGrayscale(QRgb pix)
{
    return 0.299 * qRed(pix) + 0.587 * qGreen(pix) + 0.014 * qBlue(pix);
}

void skeletonVisualization::updateImage()
{
    if (ready)
    {

    QGraphicsScene *newScene = new QGraphicsScene;

    // draw image
    if (drawImage)
    {
        QPixmap newPixmap = QPixmap::fromImage(image);
        if (scale != 1)
            newPixmap = newPixmap.scaled(newPixmap.size() * scale,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
        newScene->addPixmap(newPixmap);
    }

    // draw image
    if (drawContours)
    {
        QPen pen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

        for (uint i = 0; i < skeleton.contours.size(); i++)
        {
            for (uint j = 0; j < skeleton.contours[i].corners.size() - 1; j++)
                newScene->addLine(
                            scale * skeleton.contours[i].corners[j].x,
                            scale * skeleton.contours[i].corners[j].y,
                            scale * skeleton.contours[i].corners[j + 1].x,
                            scale * skeleton.contours[i].corners[j + 1].y,
                            pen);

            newScene->addLine(
                        scale * skeleton.contours[i].corners[0].x,
                        scale * skeleton.contours[i].corners[0].y,
                        scale * skeleton.contours[i].corners
                                [skeleton.contours[i].corners.size() - 1].x,
                        scale * skeleton.contours[i].corners
                                [skeleton.contours[i].corners.size() - 1].y,
                        pen);
        }
    }

    // draw skeleton (bones & circles)
    for (uint i = 0; i < skeleton.components.size(); i++)
    {
        // draw bones
        if (drawBones)
        {
            QPen pen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            for (uint j = 0; j < skeleton.components[i].bones.size(); j++)
                newScene->addLine(
                            scale * skeleton.components[i].nodes
                                [skeleton.components[i].bones[j].i].x + 1,
                            scale * skeleton.components[i].nodes
                                [skeleton.components[i].bones[j].i].y + 1,
                            scale * skeleton.components[i].nodes
                                [skeleton.components[i].bones[j].j].x + 1,
                            scale * skeleton.components[i].nodes
                                [skeleton.components[i].bones[j].j].y + 1,
                            pen);
        }

        // draw circles
        if (drawCircles)
        {
            QPen pen(Qt::green, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

            for (uint j = 0; j < skeleton.components[i].nodes.size(); j++)
                newScene->addEllipse(scale * (skeleton.components[i].nodes[j].x
                                              + 1 - skeleton.components[i].
                                              nodes[j].r),
                                     scale * (skeleton.components[i].nodes[j].y
                                              + 1 - skeleton.components[i].
                                              nodes[j].r),
                                     2*scale*skeleton.components[i].nodes[j].r,
                                     2*scale*skeleton.components[i].nodes[j].r,
                                     pen);
        }
    }

    ui.imageView->setScene(newScene);
    if (scene)
        delete scene;
    scene = newScene;

    }
}

void skeletonVisualization::openImage()
{
    imagepath = QFileDialog::getOpenFileName(this, "Open image", "",
                                             "Image (*.png *.jpg *.bmp)");
    if (!scene)
    {
        ui.buttonSave->setEnabled(true);
        ui.buttonRefresh->setEnabled(true);
        ui.buttonConnect->setEnabled(true);
        ui.checkBoxBones->setEnabled(true);
        ui.checkBoxImage->setEnabled(true);
        ui.checkBoxContours->setEnabled(true);
        ui.checkBoxCircles->setEnabled(true);
        ui.actionConnect->setEnabled(true);
        ui.actionRefresh->setEnabled(true);
        ui.actionSave->setEnabled(true);
        ui.menuScale->setEnabled(true);
        ui.menuView->setEnabled(true);
        ui.sliderScale->setEnabled(true);
    }
    if (imagepath.size())
    {
        if (scene)
        {
            QMessageBox msg;
            msg.setWindowTitle("Open new image");
            msg.setText(
                   "Do you want to save the old image before opening new one?");
            msg.setIcon(QMessageBox::Question);
            msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
            msg.setDefaultButton(QMessageBox::Discard);
            if (msg.exec() == QMessageBox::Save)
                saveImage();
        }
        skeletonView = scale = 1;
        updateSkeleton();
    }
}

void skeletonVisualization::checkBoxesChanged(int)
{
    bool flag = false;
    if (ready)
    {
        ready = false;
        flag = true;
    }

    drawBones = (ui.checkBoxBones->checkState() == Qt::Checked);
    drawCircles = (ui.checkBoxCircles->checkState() == Qt::Checked);
    drawContours = (ui.checkBoxContours->checkState() == Qt::Checked);
    drawImage = (ui.checkBoxImage->checkState() == Qt::Checked);

    if (flag)
        ready = true;

    updateImage();
}

void skeletonVisualization::updateSkeleton()
{
    if (imagepath.size() != 0)
    {
		image = QImage(imagepath);

		byte* img = new byte[image.height() * image.width()];

        for (int i = 0; i < image.height(); i++)
            for(int j = 0; j < image.width(); j++)
                if (skeletonView)
                    img[i * image.width() + j] =
                        toGrayscale(image.pixel(j, image.height() - 1 - i));
                else
                    img[i * image.width() + j] = 255 -
                        toGrayscale(image.pixel(j, image.height() - 1 - i));

		SkeletonMaker::SourceImage srcimg;
		srcimg.height = image.height();
		srcimg.width = image.width();
		srcimg.pixels = img;

        skeleton = skeletonMaker.createSkeleton(&srcimg,
                                                ui.spinBoxPruning->value(),
                                                ui.spinBoxArea->value());

        ui.labelTime->setText(
                    QString("Elapsed time:\n%1 ms - DLL\n%2 ms - C++").
                    arg(skeleton.totalTime).arg(skeleton.cppTime));

        ready = true;
        updateImage();
	}
}

void skeletonVisualization::saveImage()
{
    QString path = QFileDialog::getSaveFileName(this, "Save image", "",
                                                "Image (*.png)");
    if (path.size())
    {
        QImage image(ui.imageView->scene()->width(),
                     ui.imageView->scene()->height(),
                     QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter painter(&image);
        ui.imageView->scene()->render(&painter);

        image.save(path);
    }
}

void skeletonVisualization::breaksConnector()
{
    this->close();
}

void skeletonVisualization::exitMethod()
{
    if (scene)
    {
        QMessageBox msg;
        msg.setWindowTitle("Exit");
        msg.setText("Do you want to save the image before exit?");
        msg.setIcon(QMessageBox::Question);
        msg.setStandardButtons(QMessageBox::Save |
                               QMessageBox::Discard | QMessageBox::Cancel);
        msg.setDefaultButton(QMessageBox::Cancel);
        switch (msg.exec())
        {
        case QMessageBox::Save:
            saveImage();
        case QMessageBox::Discard:
            this->close();
        }
    }
    else
        this->close();
}

void skeletonVisualization::setScaleValue(int val)
{
    scale = float(val) / 100;
    updateImage();
}

void skeletonVisualization::scaleOn()
{
    if (scale <= 4.5)
        scale += 0.5;
    else
        scale = 5;
    ui.sliderScale->setValue(scale * 100);
    updateImage();
}

void skeletonVisualization::scaleOff()
{
    if (scale >= 0.7)
        scale -= 0.5;
    else
        scale = 0.2;
    ui.sliderScale->setValue(scale * 100);
    updateImage();
}

void skeletonVisualization::scaleOrig()
{
    if (scale != 1)
    {
        scale = 1;
        ui.sliderScale->setValue(100);
        updateImage();
    }
}

void skeletonVisualization::internal()
{
    skeletonView = true;
    updateSkeleton();
}

void skeletonVisualization::external()
{
    skeletonView = false;
    updateSkeleton();
}
