#include "qdocktile.h"
#include "DockCircularProgressBar.h"

class QDockTilePriv {

public:
    DockCircularProgressBar *dock;
    QDockTilePriv(DockCircularProgressBar *d):dock(d){
    }
};

QDockTile::QDockTile():priv(new QDockTilePriv([DockCircularProgressBar sharedDockCircularProgressBar]))
{
    
}

QDockTile::~QDockTile()
{
    delete priv;
    priv = nullptr;
}

void QDockTile::updateProgressBar(){
    [priv->dock updateProgressBar];
}

void QDockTile::hideProgressBar(){
    [priv->dock hideProgressBar];
}


void QDockTile::setIndeterminate(bool indeterminate){
    [priv->dock setIndeterminate:indeterminate];
}


void QDockTile::setProgress(double progress){
    [priv->dock setProgress:progress];
}


void QDockTile::setIndicateString(const char *aString){
    [priv->dock setIndicateString:@(aString)];
}

void QDockTile::setIndicateNunber(int number)
{
    [priv->dock setIndicateNunber:number];
}

void QDockTile::clear(){
    [priv->dock clear];
}

void QDockTile::requestUserAttention(){
    [NSApp requestUserAttention:NSInformationalRequest];
}

static QDockTile*qDockTile = nullptr;

QDockTile *QDockTile::sharedQDockTile()
{
    if(qDockTile == nullptr){
        qDockTile = new QDockTile();
    }
    return qDockTile;
}

