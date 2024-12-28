#include "qdocktile.h"

QDockTile::QDockTile():priv(nullptr)
{
    
}

QDockTile::~QDockTile()
{
    
}

void QDockTile::updateProgressBar(){
    
}

void QDockTile::hideProgressBar(){
    
}


void QDockTile::setIndeterminate(bool indeterminate){
    
}


void QDockTile::setProgress(double progress){
    
}


void QDockTile::setIndicateString(const char *aString){
    
}

void QDockTile::setIndicateNunber(int number)
{
    
}

void QDockTile::clear(){
    
}

void QDockTile::requestUserAttention(){
    
}

static QDockTile*qDockTile = nullptr;

QDockTile *QDockTile::sharedQDockTile()
{
    if(qDockTile == nullptr){
        qDockTile = new QDockTile();
    }
    return qDockTile;
}

