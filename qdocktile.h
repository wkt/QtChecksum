#ifndef DOCKTILEWIDGET_H
#define DOCKTILEWIDGET_H


class QDockTilePriv;

class QDockTile
{
private:
    QDockTilePriv *priv;

public:
    static QDockTile *sharedQDockTile();

    QDockTile();
    ~QDockTile();

    void updateProgressBar();

    void hideProgressBar();

    // Indicates whether the progress indicator should be in an indeterminate state
    // or not.
    void setIndeterminate(bool indeterminate);
    
    // Indicates the amount of progress made of the download. Ranges from [0..1].
    void setProgress(double progress);

    // Indicates whether the progress number should be showed in circular process bar.
    void setIndicateString(const char *aString);

    void setIndicateNunber(int number);

    void requestUserAttention();

    void clear();
};

#endif // DOCKTILEWIDGET_H
