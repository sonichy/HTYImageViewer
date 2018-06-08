#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QFileInfoList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);

private:
    Ui::MainWindow *ui;
    QString path, dirTrash, dirTrashInfo;
    int index;
    QFileInfoList fileInfoList;
    void open(QString filepath);
    void genList(QString spath);
    enum {
        ZoomOriginal,
        ZoomFit
    } zoomType;
    void loadImage(QString spath);

private slots:
    void on_action_open_triggered();
    void on_action_about_triggered();
    void on_actionZoom1_triggered();
    void on_actionZoomFit_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionTrash_triggered();
    void on_actionSetWallpaper_triggered();
    void enterFullscreen();
    void exitFullscreen();
    void EEFullscreen();
    void lastImage();
    void nextImage();
    void rotate(qreal degrees);

};

#endif // MAINWINDOW_H
