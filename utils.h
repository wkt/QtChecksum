#ifndef UTILS_H
#define UTILS_H


extern const unsigned long long BYTE;
extern const unsigned long long KB;
extern const unsigned long long MB;
extern const unsigned long long GB;
extern const unsigned long long TB;
extern const unsigned long long PB;


#ifdef QT_CORE_LIB
#include<QString>
#include<QStringList>


QString sizeToString(unsigned long long size);

#if defined (__APPLE__) && __APPLE__
typedef  void (*OnFinderHandleFiles)(const QStringList &files);
void setupFinderService(OnFinderHandleFiles func);
#endif

#endif //QT_CORE_LIB

#define DIGEST_FORAMT_LOWER_CASE_HEX (0)

#define DIGEST_FORAMT_UPPER_CASE_HEX (1)

#define DIGEST_FORAMT_BASE64 (2)

#define QT_APPLICATION_NAME  "QtChecksum"
#define QT_APPLICATION_NAME_LOWER  "qtchecksum"

#define QT_ORGANIZATION_NAME "com.weiketing." QT_APPLICATION_NAME
#define KEY_ENABLE_FILE_MENU_ITEM  "enableFileMenuItem"

#ifdef __cplusplus
extern "C" {
#endif

int random_bounded(int a,int b);
int  is_gnome_desktop();

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
