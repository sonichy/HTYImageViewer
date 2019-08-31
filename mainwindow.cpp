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
#include <QLineEdit>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    ui->setupUi(this);
    ui->centralWidget->setStyleSheet("background:black; border:none;");
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
    path = "";
    dir = ".";
    index = -1;
    scale = 1.0;
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
    LSB4 = new QLabel("100%");
    LSB4->setMinimumSize(20,20);
    LSB4->setStyleSheet("padding:0px 3px;");
    ui->statusBar->addWidget(LSB1);
    ui->statusBar->addWidget(LSB2);
    ui->statusBar->addWidget(LSB3);
    ui->statusBar->addWidget(LSB4);

    label_info = new QLabel(this);
    label_info->move(0, ui->menuBar->height() + ui->mainToolBar->height());
    label_info->setStyleSheet("color:rgb(255,255,255); background:rgba(255,255,255,30);");
    //label_info->setAttribute(Qt::WA_TranslucentBackground,true);
    //label_info->setAutoFillBackground(true);
    label_info->hide();

    movie = new QMovie;
    connect(movie,SIGNAL(frameChanged(int)),this,SLOT(frameChange(int)));

    timer = new QTimer(this);
    timer->setInterval(2000);
    connect(timer, SIGNAL(timeout()), this, SLOT(autoPlay()));

    connect(ui->action_quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(new QShortcut(QKeySequence(Qt::Key_Return),this), SIGNAL(activated()), this, SLOT(EEFullScreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Enter),this), SIGNAL(activated()), this, SLOT(EEFullScreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()), this, SLOT(exitFullScreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Left),this), SIGNAL(activated()), this, SLOT(lastImage()));
    connect(new QShortcut(QKeySequence(Qt::Key_Right),this), SIGNAL(activated()), this, SLOT(nextImage()));
    connect(new QShortcut(QKeySequence(Qt::Key_R),this), SIGNAL(activated()), this, SLOT(on_actionRotateRight_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_L),this), SIGNAL(activated()), this, SLOT(on_actionRotateLeft_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_1),this), SIGNAL(activated()), this, SLOT(on_actionZoom1_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_2),this), SIGNAL(activated()), this, SLOT(on_actionZoomBig_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_3),this), SIGNAL(activated()), this, SLOT(on_actionZoomFit_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_I),this), SIGNAL(activated()), this, SLOT(on_actionInfo_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_F5),this), SIGNAL(activated()), this, SLOT(on_actionPlay_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Delete),this), SIGNAL(activated()), this, SLOT(on_actionTrash_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Space),this), SIGNAL(activated()), this, SLOT(playPause()));
    connect(new QShortcut(QKeySequence(Qt::Key_F2),this), SIGNAL(activated()), this, SLOT(on_action_rename_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Plus),this), SIGNAL(activated()), this, SLOT(zoomIn()));
    connect(new QShortcut(QKeySequence(Qt::Key_Minus),this), SIGNAL(activated()), this, SLOT(zoomOut()));

    QAction *labelAction_edit = new QAction("编辑&E", this);
    labelAction_edit->setIcon(QIcon::fromTheme("edit"));
    connect(labelAction_edit, &QAction::triggered, [=](){
        QString image_editor = settings.value("image_editor").toString();
        QFileInfo fileInfo(image_editor);
        if(fileInfo.isFile()){
            QProcess *process = new QProcess;
            QStringList arguments;
            arguments << path;
            qDebug() << image_editor << path;
            process->start(image_editor, arguments);
        }
    });

    QAction *labelAction_copyto = new QAction("复制到&C", this);
    labelAction_copyto->setIcon(QIcon::fromTheme("edit-copy"));
    connect(labelAction_copyto, &QAction::triggered, [=](){
        dir = settings.value("dir").toString();
        dir = QFileDialog::getExistingDirectory(NULL, "复制到", dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(dir != "") copy(path, dir, false);
    });

    QAction *labelAction_moveto = new QAction("移动到&X", this);
    labelAction_moveto->setIcon(QIcon::fromTheme("edit-cut"));
    connect(labelAction_moveto, &QAction::triggered, [=](){
        dir = settings.value("dir").toString();
        dir = QFileDialog::getExistingDirectory(NULL, "移动到", dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if(dir != "") copy(path, dir, true);
    });

    QMenu *labelMenu = new QMenu;
    labelMenu->addAction(labelAction_edit);
    labelMenu->addAction(labelAction_copyto);
    labelMenu->addAction(labelAction_moveto);
    ui->label->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->label, &QLabel::customContextMenuRequested, [=](){
        labelMenu->exec(QCursor::pos());
    });

    QStringList SLargs = QApplication::arguments();
    qDebug() << SLargs;
    if (SLargs.length() > 1) {
        QUrl url(SLargs.at(1));
        open(url.toLocalFile());
    }

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_open_triggered()
{
    if (path == "") path = ".";
    path = QFileDialog::getOpenFileName(this,"打开图片", path, "图片文件(*.jpg *.jpeg *.png *.bmp *.gif *.svg)");
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
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰看图 1.2\n一款基于Qt的看图程序。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy");
    aboutMB.setIconPixmap(QPixmap(":/HTYIV.png"));
    aboutMB.exec();
}

void MainWindow::on_action_settings_triggered()
{
    if(dialog_set != NULL){
        dialog_set = new QDialog(this);
        dialog_set->setWindowTitle("设置");
        dialog_set->setFixedSize(400,300);
        QGridLayout *gridLayout = new QGridLayout;
        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnStretch(1, 3);
        QLabel *label = new QLabel("图片编辑器");
        label->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(label,0,0,Qt::AlignTop);
        QPushButton *pushButton_image_editor = new QPushButton;
        pushButton_image_editor->setStyleSheet("text-align:right;");
        QString image_editor = settings.value("image_editor").toString();
        pushButton_image_editor->setText(image_editor);
        pushButton_image_editor->setToolTip(image_editor);
        connect(pushButton_image_editor, &QPushButton::clicked, [=](){
            QString path1 = QFileDialog::getOpenFileName(this, "选择图片编辑器", image_editor, "可执行程序(*)");
            if(path1 != ""){
                pushButton_image_editor->setText(path1);
                pushButton_image_editor->setToolTip(path1);
                settings.setValue("image_editor", path1);
            }
        });
        gridLayout->addWidget(pushButton_image_editor,0,1,Qt::AlignTop);
        dialog_set->setLayout(gridLayout);
        dialog_set->exec();
    }
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

void MainWindow::enterFullScreen()
{
    //windowState = windowState();
    showFullScreen();
    ui->menuBar->hide();
    ui->mainToolBar->hide();
    ui->statusBar->hide();
    //setCursor(QCursor(Qt::BlankCursor));
    //PMAFullscreen->setText("退出全屏");
}

void MainWindow::exitFullScreen()
{
    showNormal();
    //setCursor(QCursor(Qt::ArrowCursor));
    ui->menuBar->show();
    ui->mainToolBar->show();
    ui->statusBar->show();
    //PMAFullscreen->setText("全屏");
    timer->stop();
}

void MainWindow::EEFullScreen()
{
    if (isFullScreen()) {
        exitFullScreen();
    } else {
        enterFullScreen();
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
    if (ui->scrollArea->horizontalScrollBar()->visibleRegion().isEmpty()) { // 横向滚动条没有显示
        int id = index - 1;
        if (id > -1) {
            path = fileInfoList.at(id).absoluteFilePath();
            scale = 1.0;
            loadImage(path);
            index = id;
            LSB2->setText(QString::number(id+1) + "/" + QString::number(fileInfoList.size()));
            LSB3->setText(BS(QFileInfo(path).size()) + " " + QFileInfo(path).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        }
    } else {
        ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() - 20);
    }
}

void MainWindow::nextImage()
{
    if (ui->scrollArea->horizontalScrollBar()->visibleRegion().isEmpty()) {
        int id = index + 1;
        if (id < fileInfoList.size()) {
            path = fileInfoList.at(id).absoluteFilePath();
            scale = 1.0;
            loadImage(path);
            index = id;
            LSB2->setText(QString::number(id+1) + "/" + QString::number(fileInfoList.size()));
            LSB3->setText(BS(QFileInfo(path).size()) + " " + QFileInfo(path).lastModified().toString("yyyy-MM-dd hh:mm:ss"));
        }
    } else {
        ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() + 20);
    }
}

void MainWindow::on_actionZoom1_triggered()
{
    if(ui->label->pixmap() != 0){
        zoomType = ZoomOriginal;
        loadImage(fileInfoList.at(index).absoluteFilePath());
    }
}

void MainWindow::on_actionZoomBig_triggered()
{
    if(ui->label->pixmap() != 0){
        zoomType = ZoomBig;
        loadImage(fileInfoList.at(index).absoluteFilePath());
    }
}

void MainWindow::on_actionZoomFit_triggered()
{
    if(ui->label->pixmap() != 0){
        zoomType = ZoomFit;
        loadImage(fileInfoList.at(index).absoluteFilePath());
    }
}

void MainWindow::on_actionRotateLeft_triggered()
{
    if(ui->label->pixmap() != 0)
        rotate(-90);
}

void MainWindow::on_actionRotateRight_triggered()
{
    if(ui->label->pixmap() != 0)
        rotate(90);
}

void MainWindow::loadImage(QString spath)
{
    movie->stop();
    setWindowTitle(QFileInfo(spath).fileName());
    QString MIME = QMimeDatabase().mimeTypeForFile(spath).name();
    if(MIME == "image/gif"){
        movie->setFileName(spath);
        movie->start();
    }else{
        QImageReader reader(spath);
        reader.setAutoTransform(true);  // auto rotate image
        QImage image = reader.read();
        QImage image_zoom = image;

        QPainter painter(&image_zoom);
        //生成棋盘背景
        int dx = 50;
        int dy = 50;
        QBrush brush1(QColor(200,200,200));
        QBrush brush2(QColor(255,255,255));
        for(int y=0; y<image_zoom.height(); y+=dy){
            for(int x=0; x<image_zoom.width(); x+=dx){
                painter.fillRect(x, y, dx/2, dy/2, brush1);
                painter.fillRect(x + dx/2, y, dx/2, dy/2, brush2);
                painter.fillRect(x, y + dy/2, dx/2, dy/2, brush2);
                painter.fillRect(x + dx/2, y + dy/2, dx/2, dy/2, brush1);
            }
        }
        painter.drawImage(0,0,image);

        if(zoomType == ZoomFit){
            image_zoom = image_zoom.scaled(ui->centralWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            scale = ui->centralWidget->width()/image.width();
        }else if(zoomType == ZoomBig){
            if(image.width() > ui->centralWidget->width() || image.height() > ui->centralWidget->height()){
                image_zoom = image_zoom.scaled(ui->centralWidget->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                scale = ui->centralWidget->width()/image.width();
            }
        }else if(zoomType == ZoomOriginal){
            scale = 1;
        }else if(zoomType == ZoomManual){
            image_zoom = image_zoom.scaled(image.size() * scale, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        ui->label->setPixmap(QPixmap::fromImage(image_zoom));
        LSB1->setText("分辨率：" + QString::number(image.width()) + " X " + QString::number(image.height()));
        LSB4->setText(QString::number(image_zoom.width()*100/image.width()) + "%");

        //qDebug() << reader.textKeys();
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
    if(ui->label->pixmap() !=0){
#ifdef Q_OS_LINUX
        if(!QDir(dirTrash).exists()) QDir().mkpath(dirTrash);
        if(!QDir(dirTrashInfo).exists()) QDir().mkpath(dirTrashInfo);
        QString filepath = fileInfoList.at(index).absoluteFilePath();
        QString newName = dirTrash + "/" + QFileInfo(filepath).fileName();
        if (QFile::copy(filepath, newName)) {
            QString pathinfo = dirTrashInfo + "/" + QFileInfo(filepath).fileName() + ".trashinfo";
            QFile file(pathinfo);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream stream(&file);
                QDateTime time = QDateTime::currentDateTime();
                stream << "[Trash Info]\nPath=" + filepath + "\nDeletionDate=" + time.toString("yyyy-MM-ddThh:mm:ss");
                file.close();
            }
            if (QFile::remove(filepath)) {
                genList(QFileInfo(filepath).absolutePath());
                qDebug() << index << fileInfoList.size();
                if (index >= fileInfoList.size()) index--;
                loadImage(fileInfoList.at(index).absoluteFilePath());
            } else {
                QMessageBox::critical(NULL, "错误", "无法删除文件 " + filepath);
            }
        } else {
            QMessageBox::critical(NULL, "错误", "无法移动 " + filepath + " 到回收站");
        }
#endif
    }
}

void MainWindow::on_actionSetWallpaper_triggered()
{
    if(ui->label->pixmap() !=0){
        if(index != -1){
#ifdef Q_OS_LINUX
            QString cmd = "gsettings set org.gnome.desktop.background picture-uri file://" + fileInfoList.at(index).absoluteFilePath();
            qDebug() << cmd;
            QProcess *process = new QProcess;
            process->start(cmd);
#endif
        }
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
    LSB1->setText("分辨率：" + QString::number(pixmap.width()) + " X " + QString::number(pixmap.height()) + "  帧：" + QString::number(fn) + "/" + QString::number(movie->frameCount()-1));
    LSB4->setText(QString::number(pixmap_zoom.width()*100/pixmap.width()) + "%");
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    EEFullScreen();
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

void MainWindow::on_actionPlay_triggered()
{
    enterFullScreen();
    timer->start();
}

void MainWindow::autoPlay()
{
    if(index > fileInfoList.size()-1)
        index = 0;
    nextImage();
}

void MainWindow::playPause()
{
    if(timer->isActive()){
        timer->stop();
    }else{
        timer->start();
    }
}

void MainWindow::on_action_rename_triggered()
{
    if(ui->label->pixmap() !=0){
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle("重命名");
        QVBoxLayout *vbox = new QVBoxLayout;
        QLineEdit *lineEdit = new QLineEdit;
        lineEdit->setText(QFileInfo(path).baseName());
        lineEdit->setCursorPosition(0);
        vbox->addWidget(lineEdit);
        QHBoxLayout *hbox = new QHBoxLayout;
        QPushButton *pushButtonConfirm = new QPushButton("确定");
        QPushButton *pushButtonCancel = new QPushButton("取消");
        hbox->addWidget(pushButtonConfirm);
        hbox->addWidget(pushButtonCancel);
        vbox->addLayout(hbox);
        dialog->setLayout(vbox);
        connect(pushButtonConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(pushButtonCancel, SIGNAL(clicked()), dialog, SLOT(reject()));
        if(dialog->exec() == QDialog::Accepted){
            setWindowTitle(lineEdit->text() + "."+ QFileInfo(path).suffix());
            QString newName = QFileInfo(path).absolutePath() + "/" + lineEdit->text() + "."+ QFileInfo(path).suffix();
            qDebug() << "rename" << path << newName;
            if (QFile::rename(path, newName)) {
                path = newName;
                setWindowTitle(QFileInfo(path).fileName());
            }else{
                QMessageBox::critical(NULL, "错误", "无法重命名文件，该文件已存在！", QMessageBox::Ok);
            }
        }
        dialog->close();
        genList(QFileInfo(path).absolutePath());
    }
}

void MainWindow::zoomIn()
{
    zoomType = ZoomManual;
    scale += 0.1;
    loadImage(path);
}

void MainWindow::zoomOut()
{
    if(scale > 0.1){
        zoomType = ZoomManual;
        scale -= 0.1;
        loadImage(path);
    }
}

void MainWindow::wheelEvent(QWheelEvent *e)
{
    if(e->delta() > 0){
        zoomIn();
    }else{
        zoomOut();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::readSettings()
{
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::copy(QString source, QString dir, bool isCut)
{
    QString dest = dir + "/" + QFileInfo(source).fileName();
    if(QFile::copy(source, dest)){
        QFile file(dest);
        file.open(QIODevice::ReadOnly);
        //qDebug() << "修改文件时间" <<
        if(file.setFileTime(QFileInfo(source).lastModified(), QFileDevice::FileModificationTime)){

        }else{
           qDebug() << "修改文件时间失败";
        }
        if(isCut){
            if(QFile::remove(source)){

            }else{
                qDebug() << "删除源文件失败";
            }
        }
    }else{
        qDebug() << "复制失败";
    }
}