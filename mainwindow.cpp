#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QThread>
#include <QTimer>
#include <QStandardPaths>
#include <QMessageBox>
#include <QProcess>
#include <QClipboard>
#include <QTextDocument>
#include <QScrollBar>
#include <QScreen>


#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
#include <QRandomGenerator>
#endif


#include "utils.h"

#include "qtapp.h"
#include "qcksettingsdialog.h"

static const int MAX_PROGRESS_VALUE = 1000;


static inline QStringList
browseAnyFiles(QWidget *parent = nullptr,
               const QString &caption = QString(),
               const QString &dir = QString(),
               const QString &filter = QString())
{
    QFileDialog dlg (parent,caption);
    dlg.setNameFilter(filter);
    dlg.setDirectory(dir);
    dlg.setOption(QFileDialog::ReadOnly);
    dlg.setFileMode(QFileDialog::FileMode::ExistingFiles);
    if(dlg.exec())
        return dlg.selectedFiles();
    return QStringList();
}

class WindowPriv:public QObject{
//    Q_OBJECT
    
public:
    WindowPriv():QObject(),forceShowUpdateInfo(false),textFound(false){}
    ~WindowPriv(){
    }

protected:
    bool forceShowUpdateInfo;
    QAction *selectAll;
    QMenu  *helpMenu;
    QMenu  *editMenu;
    QMenu  *fileMenu;
    QAction *saveAction;
    QAction *browseAction;
    QAction *optionAction;
    QAction *findAction;
    bool textFound;
    int textCnt = 0;
    friend MainWindow;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,priv(new WindowPriv())
{

#ifdef __APPLE__
    setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint| Qt::MSWindowsFixedSizeDialogHint|Qt::Dialog);
#endif
    ui->setupUi(this);

    setAcceptDrops(true);
    _w = this->width();
    _h = this->height();
    //setFixedSize(this->width(),this->height());
    //qDebug()<<"w:"<<_w<<",h:"<<_h;
    setupMenubar();
    
    ui->fileProgressBar->setRange(0,MAX_PROGRESS_VALUE);
    ui->totalProgressBar->setRange(0,MAX_PROGRESS_VALUE);
    
    connect(ui->browseButton,&QPushButton::clicked,this,&MainWindow::actionBrowseFiles);
    connect(ui->clearButton,&QPushButton::released,this,&MainWindow::actionClearTexts);
    connect(ui->saveButton,&QPushButton::clicked,this,&MainWindow::actionSaveTexts);
    connect(ui->copyButton,&QPushButton::clicked,this,&MainWindow::actionCopyTexts);
    connect(ui->abortButton,&QPushButton::clicked,qApp,&QtApp::abortTasks);
    
    connect(ui->md5CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setMD5(bool)));
    connect(ui->sha1CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setSHA1(bool)));
    connect(ui->sha256CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setSHA256(bool)));
    connect(ui->crc32CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setCRC32(bool)));

    connect(ui->sha512CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setSHA512(bool)));
    connect(ui->sha3_256CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setSHA3_256(bool)));
    connect(ui->sha3_512CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setSHA3_512(bool)));
    connect(ui->keccak256CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setKECCAK_256(bool)));
    connect(ui->keccak512CheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setKECCAK_512(bool)));


    connect(qApp,&QtApp::onUpdateInfo,this,&MainWindow::showUpdateInfo);
    connect(qApp,&QtApp::quitRequested,this,&MainWindow::sendCloseEvent);
    
    ui->md5CheckBox->setChecked(qApp->canMD5());
    ui->sha1CheckBox->setChecked(qApp->canSHA1());
    ui->sha256CheckBox->setChecked(qApp->canSHA256());
    ui->crc32CheckBox->setChecked(qApp->canCRC32());

    ui->sha512CheckBox->setChecked(qApp->canSHA512());
    ui->sha3_256CheckBox->setChecked(qApp->canSHA3_256());
    ui->sha3_512CheckBox->setChecked(qApp->canSHA3_512());
    ui->keccak256CheckBox->setChecked(qApp->canKECCAK_256());
    ui->keccak512CheckBox->setChecked(qApp->canKECCAK_512());

    setProgress(0,0);
    setWindowIcon(QIcon(":/icons/qthash.png"));

    connect(ui->optionsButton,SIGNAL(clicked(bool)),this,SLOT(showSettingsDialog()));
    connect(ui->btnHideSearch,SIGNAL(clicked(bool)),this,SLOT(hideTextSearch()));
    connect(ui->searchEdit,SIGNAL(returnPressed()),this,SLOT(findNext()));
    connect(ui->searchEdit,SIGNAL(textChanged(const QString& ) ),this,SLOT(findInResult(const QString&)));
    connect(ui->nextButton,SIGNAL(clicked(bool)),this,SLOT( findForward() ));
    connect(ui->prevButton,SIGNAL(clicked(bool)),this,SLOT( findBackward() ));

    hideTextSearch();
    move2Center();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete priv;
}

void MainWindow::move2Center()
{
    //Only work on X and Not work on wayland
    //fucking ugly wayland
    QRect screenGeometry = qApp->primaryScreen()->geometry();
    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;
    this->move(x, y);
}

void MainWindow::about()
{
    QString aboutText = QString::asprintf(tr(
                "<center>"
                "<!--img src=\"qrc:/icons/qthash.png\" width=96 height=96></img-->"
                "<h3>QtChecksum  %s</h3>"
                "<p>QtChecksum is a hash(MD5/SHA1/SHA256/CRC32) calculator written in QT.</p>"
                "<p><a href=\"https://weiketing.com/apps/qtchecksum\">https://weiketing.com/apps/qtchecksum</a></p>"
                "<p>Copyright © 2024 - <a href=\"mailto:weikting@gmail.com\">Wei Keting</a>. All rights reserved.</p>"
                "</center><br/><br/>"
                ).toStdString().data(),VERSION);
    QMessageBox::about(this, tr("About"), aboutText);
}

void MainWindow::setupMenubar()
{
    QMenuBar *menubar = menuBar();

    QMenu  *fileMenu = menubar->addMenu("&File");
    QAction *browseAction = fileMenu->addAction(tr("&Browse..."),
                                                this,
                                                &MainWindow::actionBrowseFiles);
    fileMenu->addSeparator();

    QAction *saveAction = fileMenu->addAction(tr("&Save to..."),
                                              this,
                                              &MainWindow::actionSaveTexts);
    saveAction->setEnabled(false);

    QAction *exitAtion = fileMenu->addAction(tr("E&xit"),
                                             this,
                                             &MainWindow::close,
                                             QKeySequence::Quit);
    exitAtion->setMenuRole(QAction::QuitRole);


    QMenu  *editMenu = menubar->addMenu("&Edit");
    QAction *action = editMenu->addAction(tr("&Copy"),
                                          ui->resultTextEdit,
                                          SLOT(copy()),
                                          QKeySequence(QKeySequence::Copy));
    connect(ui->resultTextEdit,SIGNAL(copyAvailable(bool)),action,SLOT(setEnabled(bool)));
    action->setEnabled(false);

    action = editMenu->addAction(tr("Select &All"),
                                 this,
                                 &MainWindow::resultSelectAll,
                                 QKeySequence(QKeySequence::SelectAll));
    action->setEnabled(false);
    priv->selectAll = action;

    action = editMenu->addAction(tr("&Find"),
                                 this,
                                 &MainWindow::toggleTextSearch,
                                 QKeySequence(QKeySequence::Find));
    action->setEnabled(false);
    priv->findAction = action;

    QAction *optionAction = editMenu->addAction(tr("Preferences..."),
                                                this,
                                                SLOT(showSettingsDialog()),
                                                QKeySequence::Preferences);
    optionAction->setMenuRole(QAction::PreferencesRole);

    QMenu *helpMenu = menubar->addMenu("Help");

    QAction *aboutAct = helpMenu->addAction(tr("About"), this, &MainWindow::about);
    aboutAct->setMenuRole(QAction::AboutRole);

    QAction *aboutQt = helpMenu->addAction(tr("About Qt"), qApp, &QApplication::aboutQt);
    aboutQt->setMenuRole(QAction::AboutQtRole);

    QAction *checkAction = helpMenu->addAction(tr("Check Update"),this,&MainWindow::checkUpdate);
    checkAction->setStatusTip(tr("Check Fro update"));
    checkAction->setVisible(true);
    priv->helpMenu = helpMenu;
    priv->browseAction = browseAction;
    priv->saveAction = saveAction;
    priv->fileMenu = fileMenu;
    priv->editMenu = editMenu;
    priv->optionAction = optionAction;
};

void MainWindow::updateMenu()
{
   priv->helpMenu->setTitle(tr("&Help"));
   priv->fileMenu->setTitle(tr("&File"));
   priv->editMenu->setTitle(tr("&Edit"));
}

void MainWindow::resultSelectAll()
{
    ui->resultTextEdit->selectAll();
}

void MainWindow::actionBrowseFiles(){
    QVariant var = qApp->getValue(KEY_DEFAULT_PATH,QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

    QStringList files = browseAnyFiles(this,
                                       tr("Select file(s) to calculate checksum"),
                                       var.toString(),
                                       tr("All (*)"));
    if(files.length()>0){
        QFileInfo fi(files[0]);
        if(fi.isDir()){
            qApp->setValue(KEY_DEFAULT_PATH,fi.absoluteFilePath());
        }else{
            qApp->setValue(KEY_DEFAULT_PATH,fi.absolutePath());
        }
        addFiles(files);
    }
}

void MainWindow::actionClearTexts(){
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr(QT_APPLICATION_NAME));
    msgBox.setText(QObject::tr("Clear the result texts ."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setInformativeText(tr("Are you sure to continue ?"));
    msgBox.setStandardButtons(QMessageBox::Yes |QMessageBox::Cancel);
    msgBox.setEscapeButton(QMessageBox::Cancel);
    if(msgBox.exec() != QMessageBox::Yes){
        return;
    }
    
    setResultText("");
    __ugly_update();
    ui->saveButton->setEnabled(false);
    ui->empty_placeholder->setVisible(true);
    ui->fileProgressBar->setValue(0);
    ui->totalProgressBar->setValue(0);
    ui->copyButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    priv->selectAll->setEnabled(false);
    priv->saveAction->setEnabled(false);
    priv->findAction->setEnabled(false);
    setProgress(0,0);
}

void MainWindow::actionCopyTexts(){
    qApp->clipboard()->setText(ui->resultTextEdit->toPlainText());
}

void MainWindow::actionSaveTexts(){
    QVariant var = qApp->getValue(KEY_DEFAULT_PATH,QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    QString filePath = QFileInfo(QDir(var.toString()),tr("checksum.txt")).absoluteFilePath();
    QString file = QFileDialog::getSaveFileName(this,tr("Save to file"),filePath,tr("Text files (*.txt);"));
    if(file.length()>0){
        QFile qf(file);
        if(qf.open(QFile::WriteOnly)){
            const QString &txt = ui->resultTextEdit->toPlainText();
            qf.write(txt.toUtf8());
            qf.close();
        }else {
            qWarning()<<"open("<<file<<"): "<<qf.errorString();
        }
    }
}


void MainWindow::addFiles(const QStringList &files)
{
    qApp->openFiles(files);
}

void MainWindow::setCheckBoxEnabled(bool enabled)
{
    ui->checkboxLayout->setEnabled(enabled);
    ui->md5CheckBox->setEnabled(enabled);
    ui->sha1CheckBox->setEnabled(enabled);
    ui->sha256CheckBox->setEnabled(enabled);
    ui->crc32CheckBox->setEnabled(enabled);
    ui->clearButton->setEnabled(enabled);
    if(enabled){
        ui->saveButton->setEnabled(ui->resultTextEdit->toPlainText().trimmed().length()>0);
    }else{
        ui->saveButton->setEnabled(false);
    }
    priv->saveAction->setEnabled(ui->saveButton->isEnabled());
    ui->hashAlogrithmBox->setEnabled(enabled);
    ui->optionsButton->setEnabled(enabled);
    priv->optionAction->setEnabled(enabled);
}

void MainWindow::setResultText(const QString &text,bool append,bool html)
{
    QString txt = text;
#ifndef __APPLE__
    if(html){
        txt = txt.replace("<big>","<b>").replace("</big>","</b>");
    }
#endif
    ui->resultTextEdit->moveCursor(QTextCursor::End);
    if(append){
        if(html){
            ui->resultTextEdit->insertHtml(txt);
        }else{
            ui->resultTextEdit->insertPlainText(text);
        }
    }else{
        if(html){
            ui->resultTextEdit->setHtml(txt);
        }else{
            ui->resultTextEdit->setPlainText(text);
        }
    }
    ui->resultTextEdit->moveCursor(QTextCursor::End);
    
    int n_texts = ui->resultTextEdit->toPlainText().trimmed().length();
    emit resultTextAvailable(n_texts>0);
    ui->empty_placeholder->setVisible(n_texts == 0);
    if(qApp->isTasksRuning())return;
    ui->copyButton->setEnabled(n_texts>0);
    ui->clearButton->setEnabled(n_texts>0);
    priv->saveAction->setEnabled(ui->saveButton->isEnabled());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event){
    const QMimeData *mimeData = event->mimeData();
    //qDebug()<<__PRETTY_FUNCTION__<<","<<mimeData->formats();
    if(mimeData->hasUrls() && mimeData->formats().contains("text/uri-list")){
        event->acceptProposedAction();
    }else{
        event->ignore();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event){
    event->acceptProposedAction();
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event){
    event->accept();
}

void MainWindow::dropEvent(QDropEvent *event){
    const QMimeData *mimeData = event->mimeData();
    QList<QUrl> uris = mimeData->urls();
    QStringList files;
    QString path;
    for(int i=0;i<uris.size();i++){
        path = uris.at(i).path();
#ifdef _WIN32
        if(path.startsWith("/"))
            path = path.remove(0,1);
#endif
        files.append(path);
    }
    addFiles(files);
    event->accept();
};

void MainWindow::setProgress(double fileProgress,double totalProgress){
    ui->fileProgressBar->setValue(fileProgress*MAX_PROGRESS_VALUE);
    ui->totalProgressBar->setValue(totalProgress*MAX_PROGRESS_VALUE);
}

void MainWindow::updateFileProgressStarted(const QFileInfo qfile){
    const QFileInfo *file = & qfile;
    QString filePath = QDir::toNativeSeparators(file->absoluteFilePath());
    QString txt = "<code>" + tr("File:") +"&nbsp;"+filePath+"<br/></code>";
    if(file->isDir()){
        txt += ""+tr("Directory:")+"&nbsp;"+tr("yes")+"<br/>";
    }else{
        quint64 size = (quint64)file->size();
        txt += "<code>"+tr("Size:")+ QString::asprintf("&nbsp;%lld ",size)+ tr("Bytes");
        if(size>= KB){
            txt += QString::asprintf(" (%s)",sizeToString(size).toUtf8().data());
        }
        txt += "<br/></code>";
    }
    txt += "<code>"+tr("Modified:")+""
                                    "&nbsp;"+(file->lastModified().toLocalTime().toString("yyyy-MM-dd hh:mm:ss"));
    appendResultText(txt+"<br/></code>",true);
}

void MainWindow::updateFileProgressFinished(const QFileInfo file,const QString result){
    appendResultText(result,true);
    __ugly_update();
    ui->resultTextEdit->ensureCursorVisible();
}

void MainWindow::updateProgressStarted()
{
    setCheckBoxEnabled(false);
    ui->abortButton->setEnabled(true);
    ui->copyButton->setEnabled(false);
    ui->clearButton->setEnabled(false);
    priv->selectAll->setEnabled(false);
    priv->findAction->setEnabled(false);
    setProgress(0,0);
}

void MainWindow::updateProgressFinished()
{
    //qDebug()<<__PRETTY_FUNCTION__<<",thread: "<<QThread::currentThread();
    setCheckBoxEnabled(true);
    ui->abortButton->setEnabled(false);
    
    int n_texts = ui->resultTextEdit->toPlainText().trimmed().length();
    bool has_text = n_texts>0;
    ui->copyButton->setEnabled(has_text);
    ui->clearButton->setEnabled(has_text);
    priv->selectAll->setEnabled(has_text);
    priv->findAction->setEnabled(has_text);
}

//It just works, so have to use it.
//make QEditText update text normally
void MainWindow::__ugly_update(){
    int w = this->width();
    int h = this->height();
    int dw = 0;
    int dh = 0;
    while(dw == 0 && dh == 0){
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        dw = w <= _w ? 1:QRandomGenerator::global()->bounded(-1,1);
        dh = h <= _h ? 1:QRandomGenerator::global()->bounded(-1,1);
#else
        dw = w <= _w ? 1:random_bounded(-1,2);
        dh = h <= _h ? 1:random_bounded(-1,2);
#endif
        //qDebug()<<"dw: "<<dw<<",dh: "<<dh<<"\n";
    }
    resize(w+dw,h+dh);
}

void MainWindow::ensureHashEnabled(){
    bool checked = ui->md5CheckBox->isChecked() || ui->sha1CheckBox->isChecked() || ui->sha256CheckBox->isChecked()
            || ui->crc32CheckBox->isChecked() || ui->sha512CheckBox->isChecked() || ui->sha3_256CheckBox->isChecked()
            || ui->sha3_512CheckBox->isChecked() || ui->keccak256CheckBox->isChecked() || ui->keccak256CheckBox->isChecked();
    if(!checked){
        ui->sha256CheckBox->setChecked(true);
    }
}

void MainWindow::showUpdateInfo(QJsonObject json)
{
    bool show = false;
    QString ti = tr("Up To Date");
    QString msg = "";
    QString ver = json.value(KEY_VER).toString();
    if(ver.size()>0){
        ti= tr("Found new version: ")+ver;
        msg = tr("Changes:") +"\n    "+json.value(KEY_LOG).toString().replace("\n","\n    ");
        show = true;
    }
    if(priv->forceShowUpdateInfo){
        show = true;
    }

    priv->forceShowUpdateInfo = false;
    if(!show){
        return;
    }
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr(QT_APPLICATION_NAME));
    msgBox.setWindowFlags((msgBox.windowFlags()& ~Qt::WindowType_Mask)|Qt::Sheet);
    msgBox.setText(ti);
    QMessageBox::StandardButtons sb = QMessageBox::Cancel;
    msgBox.setIcon(QMessageBox::Information);
    if(msg.size()>0){
        msgBox.setDetailedText(msg);
    }else{
        sb = QMessageBox::Ok;
        msgBox.setInformativeText(tr("No update(s) found"));
    }
    QString url = json.value(KEY_URL).toString();
    if(url.trimmed().size()>=8){
        sb = sb | QMessageBox::QMessageBox::Open;
    }
    msgBox.setStandardButtons(sb);
    if (sb & QMessageBox::Open)
        msgBox.button(QMessageBox::Open)->setText(tr("Update"));
    msgBox.setTextFormat(Qt::RichText);
    int ret = msgBox.exec();
    //qDebug()<<"ret: "<<ret<<"\n";
    if(ret == QMessageBox::Open){
        QProcess::execute("open",QStringList(url));
    }
}

void MainWindow::checkUpdate()
{
    priv->forceShowUpdateInfo = true;
    qApp->checkUpdate();
}


void MainWindow::closeEvent(QCloseEvent *event){
    if(qApp->isTasksRuning()){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr(QT_APPLICATION_NAME));
        msgBox.setWindowFlags((msgBox.windowFlags()& ~Qt::WindowType_Mask)|Qt::Sheet);
        msgBox.setText(tr("Application is busying"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setInformativeText(tr("Are you sure to quit now ?"));
        msgBox.setStandardButtons(QMessageBox::Yes |QMessageBox::No);
        msgBox.setEscapeButton(QMessageBox::No);
        if(msgBox.exec() != QMessageBox::Yes){
            event->ignore();
            return;
        }
    }
    qApp->abortTasks();
    QMainWindow::closeEvent(event);
}

void MainWindow::sendCloseEvent()
{
    QEvent event = QEvent(QEvent::Close);
    qApp->sendEvent(this,&event);
    if(event.isAccepted()){
        qApp->quit();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
//    qDebug()<<"before:"<<tr("Up To Date");
//    qDebug()<<"event:"<<event;
    QMainWindow::changeEvent(event);
//    qDebug()<<"after:"<<tr("Up To Date")<<"\n";
    if (event->type() == QEvent::ActivationChange)
        resizeEvent(nullptr);
}

void MainWindow::resizeEvent(QResizeEvent *event){
    if (event != nullptr)
        QMainWindow::resizeEvent(event);
    ui->empty_placeholder->setScaledContents(true);
    ui->empty_placeholder->adjustSize();
    int lw = ui->empty_placeholder->width();
    int lh = ui->empty_placeholder->height();
    int fw = ui->textFrame->width();
    int fh = ui->textFrame->height();
    int x = (fw-lw)/2;
    int y = (fh-lh)/2;
    //qDebug()<<__PRETTY_FUNCTION__<< ": event:"<<event<<";lw:"<<lw<<",lh:"<<lh<<",fw:"<<fw<<",fh:"<<fh;
    if (ui->empty_placeholder->x()!=x || ui->empty_placeholder->y()!=y)
        ui->empty_placeholder->move(x,y);
}

void MainWindow::showSettingsDialog()
{
    QCKSettingsDialog dialog(this);
    dialog.setWindowTitle(tr("Preferences"));
    dialog.setWindowModality(Qt::WindowModal);
    dialog.show();
    dialog.exec();
    dialog.close();
}

void MainWindow::hideTextSearch()
{
    priv->textFound = false;
    priv->textCnt = 0;
    ui->searchWidget->hide();
    ui->searchEdit->clearFocus();
}

void MainWindow::showTextSearch()
{
    ui->searchWidget->show();
    ui->searchEdit->setFocus();
}

void MainWindow::toggleTextSearch()
{
    if(ui->searchWidget->isVisible()){
        hideTextSearch();
    }else{
        QString text = ui->resultTextEdit->textCursor().selectedText();
        showTextSearch();
        if(text.length()>0){
            ui->searchEdit->setText(text);
        }else{
            ui->searchEdit->clear();
        }
    }
}

bool MainWindow::findForward()
{
    bool found = ui->resultTextEdit->find(ui->searchEdit->text());
    ui->nextButton->setEnabled(found);
    ui->prevButton->setEnabled(priv->textCnt>1);
    return found;
}

bool MainWindow::findBackward()
{
    bool found = ui->resultTextEdit->find(ui->searchEdit->text(),QTextDocument::FindBackward);
    ui->prevButton->setEnabled(found);
    if(found)ui->nextButton->setEnabled(found);
    return found;
}

void MainWindow::findInResult(const QString &text)
{

    QString resultText = ui->resultTextEdit->toPlainText();
    priv->textCnt = resultText.count(text);

    QTextCursor cursor = ui->resultTextEdit->textCursor();
    QScrollBar *sBar = ui->resultTextEdit->verticalScrollBar();
    int spos = sBar->value();
    int pos = cursor.position();
    QTextCursor::MoveOperation op = QTextCursor::StartOfWord;
    if(cursor.selectedText() != text)op = QTextCursor::Start;
    ui->resultTextEdit->moveCursor(op);
    priv->textFound = ui->resultTextEdit->find(text);
    if(priv->textFound){
        ui->prevButton->setEnabled(true);
        ui->nextButton->setEnabled(true);
    }else{
        cursor.setPosition(pos);
        sBar->setValue(spos);
        ui->resultTextEdit->setTextCursor(cursor);
        ui->prevButton->setEnabled(false);
        ui->nextButton->setEnabled(false);
    }
}

void MainWindow::findNext(bool allow_wrap_around)
{
    if(!priv->textFound)return;
    if(!findForward() && allow_wrap_around && priv->textCnt>1){
        ui->resultTextEdit->moveCursor(QTextCursor::Start);
        findForward();
    }
}

void MainWindow::clearSelection()
{
    QTextCursor cur = ui->resultTextEdit->textCursor();
    if(cur.hasSelection()){
        cur.clearSelection();
        ui->resultTextEdit->setTextCursor(cur);
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton){
        isPress = true;
        winPoint = this->pos();
        pressPoint = event->globalPos();

#ifdef Q_OS_LINUX
        QtApp::setOverrideCursor(Qt::OpenHandCursor);
#endif

    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){
    if(isPress){
        QPoint distance = event->globalPos() - pressPoint;
        move(winPoint+distance);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        isPress = false;

#ifdef Q_OS_LINUX
        QtApp::restoreOverrideCursor();
#endif

    }
}
