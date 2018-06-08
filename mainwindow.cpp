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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
    path = "";
    index = -1;
    zoomType = ZoomFit;    
    dirTrash = QDir::homePath() + "/.local/share/Trash/files";
    dirTrashInfo = QDir::homePath() + "/.local/share/Trash/info/";

    connect(ui->action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(new QShortcut(QKeySequence(Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Enter),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()),this, SLOT(exitFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Left),this), SIGNAL(activated()),this, SLOT(lastImage()));
    connect(new QShortcut(QKeySequence(Qt::Key_Right),this), SIGNAL(activated()),this, SLOT(nextImage()));

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
        path = QFileDialog::getOpenFileName(this,"打开图片", ".", "图片文件(*.jpg *.jpeg *.png *.bmp)");
    } else {
        path = QFileDialog::getOpenFileName(this,"打开图片", path, "图片文件(*.jpg *.jpeg *.png *.bmp)");
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
    ui->scrollArea->setStyleSheet("background:black; border:none;");
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
    ui->scrollArea->setStyleSheet("");
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
            qDebug() << i << "/" << fileInfoList.size();
            break;
        }
    }
}

void MainWindow::lastImage()
{
    int id = index - 1;
    if (id > -1) {
        loadImage(fileInfoList.at(id).absoluteFilePath());
        index = id;
    }
}

void MainWindow::nextImage()
{
    int id = index + 1;
    if (id < fileInfoList.size()) {
        loadImage(fileInfoList.at(id).absoluteFilePath());
        index = id;
    }
}

void MainWindow::on_actionZoom1_triggered()
{
    zoomType = ZoomOriginal;
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
    QPixmap pixmap(spath);
    ui->statusBar->showMessage("分辨率：" + QString::number(pixmap.width()) + " X " +QString::number(pixmap.height()));
    if(zoomType == ZoomFit){
        pixmap = pixmap.scaled(ui->centralWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    if(isFullScreen())
        qDebug() << "zoom" << pixmap.size();
    ui->label->setPixmap(pixmap);
    setWindowTitle(QFileInfo(spath).fileName());
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if(index != -1)
        loadImage(fileInfoList.at(index).absoluteFilePath());
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