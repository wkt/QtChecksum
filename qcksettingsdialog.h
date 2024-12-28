#ifndef QCKSETTINGSDIALOG_H
#define QCKSETTINGSDIALOG_H

#include <QAbstractButton>
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class QCKSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    QCKSettingsDialog(QWidget *parent = nullptr);
    ~QCKSettingsDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);
    
private:
    Ui::Dialog *ui;
};

#endif // QCKSETTINGSDIALOG_H
