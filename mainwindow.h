#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDragEnterEvent>
#include <QFileInfoList>
#include <QPrinter>
#include <QMovie>
#include <QTimer>
#include <QSettings>

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
    void mouseDoubleClickEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent *e);
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    QLabel *LSB1, *LSB2, *LSB3, *LSB4, *label_info;
    QString path, dirTrash, dirTrashInfo, dir;
    int index;
    float scale;
    QFileInfoList fileInfoList;
    void open(QString filepath);
    void genList(QString spath);
    enum {
        ZoomOriginal,
        ZoomFit,
        ZoomBig,
        ZoomManual
    } zoomType;
    void loadImage(QString spath);
    QString BS(qint64 b);
    QMovie *movie;
    bool m_bPressed;
    QPoint m_point;
    QTimer *timer;
    void enterFullScreen();
    void readSettings();
    void copy(QString source, QString dir, bool isCut);
    QSettings settings;
    QDialog *dialog_set;

private slots:
    void on_action_open_triggered();
    void on_action_print_triggered();
    void on_action_printPreview_triggered();
    void on_action_rename_triggered();
    void on_action_about_triggered();
    void on_actionZoom1_triggered();
    void on_actionZoomBig_triggered();
    void on_actionZoomFit_triggered();
    void on_actionPlay_triggered();
    void on_actionRotateLeft_triggered();
    void on_actionRotateRight_triggered();
    void on_actionTrash_triggered();
    void on_actionSetWallpaper_triggered();
    void on_actionInfo_triggered();
    void on_action_settings_triggered();
    void EEFullScreen();
    void lastImage();
    void nextImage();
    void rotate(qreal degrees);
    void printDocument(QPrinter *printer);
    void frameChange(int fn);
    void exitFullScreen();
    void autoPlay();
    void playPause();
    void zoomIn();
    void zoomOut();

};

#endif // MAINWINDOW_H