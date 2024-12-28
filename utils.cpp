#include "utils.h"
#include <cstring>
#include <cstdlib>

const unsigned long long BYTE = 1L;
const unsigned long long KB = 1024 * BYTE;
const unsigned long long MB = 1024 * KB;
const unsigned long long GB = 1024 * MB;
const unsigned long long TB = 1024 * GB;
const unsigned long long PB = 1024 * TB;


QString sizeToString(unsigned long long size)
{
    QString s="";
    char buf[128] = {0};
    if(size>=PB){
        snprintf(buf,sizeof(buf),"%.1f PB",size*1.0/PB);
    }else if(size>=TB){
        snprintf(buf,sizeof(buf),"%.1f TB",size*1.0/TB);
    }else if(size>=GB){
        snprintf(buf,sizeof(buf),"%.1f GB",size*1.0/GB);
    }else if(size>=MB){
        snprintf(buf,sizeof(buf),"%.1f MB",size*1.0/MB);
    }else if(size>=KB){
        snprintf(buf,sizeof(buf),"%.1f KB",size*1.0/KB);
    }else {
        snprintf(buf,sizeof(buf),"%.1f Bytes",size*1.0/TB);
    }
    s.append(buf);
    return s;
}

int random_bounded(int a,int b)
{
    if(a == b)return a;
    if(a>b){
        int t = b;
        b = a;
        a = t;
    }
    double r = rand()*1.0/RAND_MAX;

    return r*(b-a)+a;

}

static int __is_gnome = -1;
int is_gnome_desktop()
{
    if(__is_gnome <= -1){
        __is_gnome = 0;
        char *e = getenv("GNOME_DESKTOP_SESSION_ID");
        QString qe;
        if(e != nullptr){
            qe = e;
            qe = qe.trimmed();
            if(qe.length()>1){
                __is_gnome = 1;
            }
        }
        if(__is_gnome == 0){
            e = getenv("XDG_CURRENT_DESKTOP");
            if(e != nullptr){
                qe = e;
                qe = qe.toLower();
                if(qe.contains("gnome")){
                    __is_gnome = 2;
                }
            }
        }

        if(__is_gnome == 0){
            e = getenv("GNOME_SHELL_SESSION_MODE");
            if(e != nullptr){
                qe = e;
                qe = qe.trimmed();
                if(qe.length()>0){
                    __is_gnome = 3;
                }
            }
        }
    }
    return __is_gnome != 0;
}
