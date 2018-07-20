#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QMimeData>
#include <QShortcut>
#include <QDateTime>
#include <QProcess>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPainter>
#include <QImageReader>
#include <QMimeDatabase>
#include <QScrollBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->centralWidget->setStyleSheet("background:black;border:none;");
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
    path = "";
    index = -1;
    zoomType = ZoomBig;
    dirTrash = QDir::homePath() + "/.local/share/Trash/files";
    dirTrashInfo = QDir::homePath() + "/.local/share/Trash/info/";
    m_bPressed = false;

    LSB1 = new QLabel("欢迎使用海天鹰看图！");
    LSB1->setMinimumSize(100,20);
    LSB1->setStyleSheet("padding:0px 3px;");
    //LS1->setFrameShape(QFrame::WinPanel);
    //LS1->setFrameShadow(QFrame::Sunken);
    LSB2 = new QLabel("1/1");
    LSB2->setMinimumSize(30,20);
    LSB2->setStyleSheet("padding:0px 3px;");
    LSB3 = new QLabel("");
    LSB3->setMinimumSize(20,20);
    LSB3->setStyleSheet("padding:0px 3px;");
    ui->statusBar->addWidget(LSB1);
    ui->statusBar->addWidget(LSB2);
    ui->statusBar->addWidget(LSB3);

    label_info = new QLabel(this);
    label_info->move(0, ui->menuBar->height() + ui->mainToolBar->height());
    label_info->setStyleSheet("color:rgb(255,255,255); background:rgba(255,255,255,30);");
    //label_info->setAttribute(Qt::WA_TranslucentBackground,true);
    //label_info->setAutoFillBackground(true);
    label_info->hide();

    movie = new QMovie;
    connect(movie,SIGNAL(frameChanged(int)),this,SLOT(frameChange(int)));

    connect(ui->action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(new QShortcut(QKeySequence(Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Enter),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()),this, SLOT(exitFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Left),this), SIGNAL(activated()),this, SLOT(lastImage()));
    connect(new QShortcut(QKeySequence(Qt::Key_Right),this), SIGNAL(activated()),this, SLOT(nextImage()));
    connect(new QShortcut(QKeySequence(Qt::Key_R),this), SIGNAL(activated()),this, SLOT(on_actionRotateRight_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_L),this), SIGNAL(activated()),this, SLOT(on_actionRotateLeft_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_1),this), SIGNAL(activated()),this, SLOT(on_actionZoom1_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_2),this), SIGNAL(activated()),this, SLOT(on_actionZoomBig_triggered()));
     connect(new QShortcut(QKeySequence(Qt::Key_3),this), SIGNAL(activated()),this, SLOT(on_actionZoomFit_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_I),this), SIGNAL(activated()),this, SLOT(on_actionInfo_triggered()));

    QStringList Largs = QApplication::arguments();
    qDebug() << Largs;
    if (Largs.length()>1) {
        QUrl url(Largs.at(1));
        open(url.toLocalFile());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_open_triggered()
{
    if (path=="") {
        path = QFileDialog::getOpenFileName(this,"打开图片", ".", "图片文件(*.jpg *.jpeg *.png *.bmp *.gif *.svg)");
    } else {
        path = QFileDialog::getOpenFileName(this,"打开图片", path, "图片文件(*.jpg *.jpeg *.png *.bmp *.gif *.svg)");
    }
    if (path.length() != 0) {
        open(path);
    }
}

void MainWindow::open(QString spath)
{
    loadImage(spath);
    path = spath;
    genList(QFileInfo(spath).absolutePath());
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰看图 1.0\n一款基于Qt的看图程序。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    //if(e->mimeData()->hasFormat("text/uri-list")) //只能打开文本文件
        e->acceptProposedAction(); //可以在这个窗口部件上拖放对象
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty())
        return ;

    QString fileName = urls.first().toLocalFile();

//    foreach (QUrl u, urls) {
//        qDebug() << u.toString();
//    }
//    qDebug() << urls.size();

    if(fileName.isEmpty())
        return;

    open(fileName);
    setFocus();
}

void MainWindow::enterFullscreen()
{
    showFullScreen();
    ui->menuBar->hide();
    ui->mainToolBar->hide();
    ui->statusBar->hide();
    //ui->scrollArea->setStyleSheet("background:black; border:none;");
    //setCursor(QCursor(Qt::BlankCursor));
    //PMAFullscreen->setText("退出全屏");
}

void MainWindow::exitFullscreen()
{
    showNormal();
    //setCursor(QCursor(Qt::ArrowCursor));
    ui->menuBar->show();
    ui->mainToolBar->show();
    ui->statusBar->show();
    //ui->scrollArea->setStyleSheet("");
    //PMAFullscreen->setText("全屏");
}

void MainWindow::EEFullscreen()
{
    if (isFullScreen()) {
        exitFullscreen();
    } else {
        enterFullscreen();
    }
}

void MainWindow::genList(QString spath)
{
    // 读取文件夹下所有文件 https://www.cnblogs.com/findumars/p/6006129.html
    qDebug() << "genList" << spath;
    QDir dir(spath);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.svg";
    dir.setNameFilters(filters);
    dir.setSorting(QDir::Name);
    fileInfoList.clear();
    fileInfoList = dir.entryInfoList();
    for (int i = 0; i < fileInfoList.size(); i++) {
        QFileInfo fileInfo = fileInfoList.at(i);
        //qDebug() << fileInfo.absoluteFilePath() << path;
        if (fileInfo.absoluteFilePath() == path) {
            index = i;
            //qDebug() << i << "/" << fileInfoList.size();
            LSB2->setText(QString::number(i+1) + "/" + QString::number(fileInfoList.size()));
            LSB3->setText(BS(QFileInfo(path).size()) + " " + QFileInfo(path).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
            break;
        }
    }
}

void MainWindow::lastImage()
{
    int id = index - 1;
    if (id > -1) {
        path = fileInfoList.at(id).absoluteFilePath();
        loadImage(path);
        index = id;
        LSB2->setText(QString::number(id+1) + "/" + QString::number(fileInfoList.size()));
        LSB3->setText(BS(QFileInfo(path).size()) + " " + QFileInfo(path).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    }
}

void MainWindow::nextImage()
{
    int id = index + 1;
    if (id < fileInfoList.size()) {
        path = fileInfoList.at(id).absoluteFilePath();
        loadImage(path);
        index = id;
        LSB2->setText(QString::number(id+1) + "/" + QString::number(fileInfoList.size()));
        LSB3->setText(BS(QFileInfo(path).size()) + " " + QFileInfo(path).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
    }
}

void MainWindow::on_actionZoom1_triggered()
{
    zoomType = ZoomOriginal;
    loadImage(fileInfoList.at(index).absoluteFilePath());
}

void MainWindow::on_actionZoomBig_triggered()
{
    zoomType = ZoomBig;
    loadImage(fileInfoList.at(index).absoluteFilePath());
}

void MainWindow::on_actionZoomFit_triggered()
{
    zoomType = ZoomFit;
    loadImage(fileInfoList.at(index).absoluteFilePath());
}

void MainWindow::on_actionRotateLeft_triggered()
{
    rotate(-90);
}

void MainWindow::on_actionRotateRight_triggered()
{
    rotate(90);
}

void MainWindow::loadImage(QString spath)
{
    movie->stop();
    setWindowTitle(QFileInfo(spath).fileName());
    QString MIME = QMimeDatabase().mimeTypeForFile(spath).name();
    if(MIME == "image/gif"){
        movie->setFileName(spath);
        //ui->label->clear();
        //ui->label->setMovie(movie);
        movie->start();
    }else{
        QImageReader reader(spath);
        reader.setAutoTransform(true);
        QImage image = reader.read();
        QImage image_zoom = image;
        if(zoomType == ZoomFit){
            image_zoom = image.scaled(ui->centralWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }else if(zoomType == ZoomBig){
            if(image.width() > ui->centralWidget->width() || image.height() > ui->centralWidget->height())
                image_zoom = image.scaled(ui->centralWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        ui->label->setPixmap(QPixmap::fromImage(image_zoom));
        LSB1->setText("分辨率：" + QString::number(image.width()) + " X " +QString::number(image.height()) + " " + QString::number(image_zoom.width()*100/image.width()) + "%");

        qDebug() << reader.textKeys();
        QStringList SL = reader.textKeys();
        QString s = "";
        for(int i=0; i<SL.size(); i++){
            s += SL.at(i) + "\t" + reader.text(SL.at(i)) + "\n";
        }
        //qDebug() << s;
        label_info->setText(s);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(index != -1) loadImage(fileInfoList.at(index).absoluteFilePath());
    label_info->resize(200, centralWidget()->height());
    //qDebug() << "label_info: " << label_info->x() << label_info->y() << label_info->size();
}

void MainWindow::rotate(qreal degrees)
{
    QMatrix matrix;
    matrix.rotate(degrees);
    QPixmap pixmap = ui->label->pixmap()->transformed(matrix, Qt::SmoothTransformation);
    ui->label->setPixmap(pixmap);
}

void MainWindow::on_actionTrash_triggered()
{
    QString filepath = fileInfoList.at(index).absoluteFilePath();
    QString newName = QDir::homePath() + "/.local/share/Trash/files/" + QFileInfo(filepath).fileName();
    if (QFile::copy(filepath, newName)) {
        QString pathinfo = QDir::homePath() + "/.local/share/Trash/info/" + QFileInfo(filepath).fileName() + ".trashinfo";
        QFile file(pathinfo);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            QDateTime time = QDateTime::currentDateTime();
            stream << "[Trash Info]\nPath=" + filepath + "\nDeletionDate=" + time.toString("yyyy-MM-ddThh:mm:ss");
            file.close();
        }
        if (QFile::remove(filepath)) {
            genList(QFileInfo(filepath).absolutePath());
            loadImage(fileInfoList.at(index).absoluteFilePath());
        } else {
            QMessageBox::critical(NULL, "错误", "无法删除文件 " + filepath);
        }
    } else {
        QMessageBox::critical(NULL, "错误", "无法移动 " + filepath + " 到回收站");
    }
}

void MainWindow::on_actionSetWallpaper_triggered()
{
    if(index != -1){
        QString cmd = "gsettings set org.gnome.desktop.background picture-uri file://" + fileInfoList.at(index).absoluteFilePath();
        qDebug() << cmd;
        QProcess *process = new QProcess;
        process->start(cmd);
    }
}

void MainWindow::on_action_print_triggered()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted){
        printDocument(&printer);
    }
}

void MainWindow::on_action_printPreview_triggered()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printDocument(QPrinter*)));
    preview.exec();
}

void MainWindow::printDocument(QPrinter *printer)
{
    QPainter painter(printer);
    QRect rect = painter.viewport();
    QImage img(path);
    QSize size = img.size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
    painter.setWindow(img.rect());
    painter.drawImage(0, 0, img);
}

QString MainWindow::BS(qint64 b)
{
    QString s = "";
    if (b > 999999999) {
        s = QString::number(b/(1024*1024*1024.0),'f',2) + " GB";
    } else {
        if (b > 999999){
            s = QString::number(b/(1024*1024.0),'f',2) + " MB";
        } else {
            if (b > 999) {
                s = QString::number(b/1024.0,'f',2) + " KB";
            } else {
                s = QString::number(b)+" B";
            }
        }
    }
    return s;
}

void MainWindow::on_actionInfo_triggered()
{
    qDebug() << label_info->isHidden();
    if(label_info->isHidden()){
        label_info->show();
        label_info->raise();
    }else{
        label_info->hide();
    }
}

void MainWindow::frameChange(int fn)
{
    QPixmap pixmap = movie->currentPixmap();
    QPixmap pixmap_zoom = pixmap;
    if(zoomType == ZoomFit){
        pixmap_zoom = pixmap.scaled(ui->centralWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    ui->label->setPixmap(pixmap_zoom);
    LSB1->setText("分辨率：" + QString::number(pixmap.width()) + " X " + QString::number(pixmap.height()) + "  帧：" + QString::number(fn) + "/" + QString::number(movie->frameCount()-1) + " "+ QString::number(pixmap_zoom.width()*100/pixmap.width()) + "%");
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    EEFullscreen();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bPressed = true;
        m_point = event->globalPos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bPressed) {
        ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() - (event->globalPos().x() - m_point.x())/20);
        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() - (event->globalPos().y() - m_point.y())/20);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_bPressed = false;
    setCursor(Qt::ArrowCursor);
}