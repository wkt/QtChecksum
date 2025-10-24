#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class WindowPriv;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:

    void about();
    void resultSelectAll();
    void actionBrowseFiles();
    void actionClearTexts();
    void actionSaveTexts();
    void actionCopyTexts();

    void updateProgressStarted();
    void setProgress(double fileProgress,double totalProgress,
                     int currentFileIndex=-1,int totalFileCount=-1);
    void updateProgressFinished();
    void updateFileProgressStarted(const QFileInfo file);
    void updateFileProgressFinished(const QFileInfo file,const QString result);

    void showUpdateInfo(QJsonObject json);
    void checkUpdate();
    void sendCloseEvent();
    void ensureHashEnabled();
    void updateMenu();
    void showSettingsDialog();

    void hideTextSearch();
    void showTextSearch();
    void toggleTextSearch();

    bool findForward();
    bool findBackward();
    void findNext(bool allow_wrap_around=true);
    //void findNext(){findNext(true);}
    void findInResult(const QString &text);

    void move2Center();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    //拖拽窗口
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void clearSelection();


private:
    Ui::MainWindow *ui;
    WindowPriv *priv;
    int _w;
    int _h;

    bool        isPress;
    QPoint      pressPoint;
    QPoint      winPoint;

    void __ugly_update();
    void setupMenubar();
    void addFiles(const QStringList &files);
    void setCheckBoxEnabled(bool enabled);

    void setResultText(const QString &text,bool append=false,bool html=false);
    inline void appendResultText(const QString &text,bool html=false){
        setResultText(text,true,html);
    }
    
signals:
    void resultTextAvailable(bool available);
};
#endif // MAINWINDOW_H
