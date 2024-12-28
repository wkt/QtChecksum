#include "qcksettingsdialog.h"
#include "./ui_qck_settings_dialog.h"
#include "qtapp.h"


QCKSettingsDialog::QCKSettingsDialog(QWidget *parent):QDialog(parent), ui(new Ui::Dialog())
{
    ui->setupUi(this);

    QStringList formats = {tr("Lowercase Hexadecimal"),tr("Uppercase Hexadecimal"),"Base64"};
    for(int i=0;i<formats.length();i++){
        ui->formatsComboBox->insertItem(i,formats[i]);
    }
    ui->formatsComboBox->setCurrentIndex(qApp->digestFormat());
    ui->fileMenuitemCheckBox->setChecked(qApp->enableFileMenuItem());


    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
#if (defined (__APPLE__) && __APPLE__) || defined (__NO_FILE_MENU_ITEM__)
    ui->fileMenuitemCheckBox->setEnabled(false);

#if defined (__NO_FILE_MENU_ITEM__)
    ui->fileMenuitemCheckBox->setChecked(false);
#endif

#endif

#if defined(__linux__)
    if(ui->fileMenuitemCheckBox->isEnabled() && !is_gnome_desktop()){
        ui->fileMenuitemCheckBox->setEnabled(false);
    }
#endif

    connect(ui->formatsComboBox,SIGNAL(currentIndexChanged(int)),qApp,SLOT(setDigestFormat(int)));
    connect(ui->fileMenuitemCheckBox,SIGNAL(toggled(bool)),qApp,SLOT(setEnableFileMenuItem(bool)));

}

QCKSettingsDialog::~QCKSettingsDialog(){
    delete ui;
}

void QCKSettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    done(ui->buttonBox->standardButton(button));
}
