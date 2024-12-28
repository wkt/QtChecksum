#ifndef QCKHASH_H
#define QCKHASH_H

#include <QCryptographicHash>
#include <QString>
class QCKHashImpl;
class QCKHash
{

private:
    QCKHashImpl *impl;
    QString _name;

public:
    enum HashType {
        CRC32 = -1,
        MD5 = QCryptographicHash::Md5,
        SHA1 = QCryptographicHash::Sha1,
        SHA256 = QCryptographicHash::Sha256,
        SHA512 = QCryptographicHash::Sha512,
        SHA3_256 = QCryptographicHash::Sha3_256,
        SHA3_512 = QCryptographicHash::Sha3_512,
        KECCAK_256 = QCryptographicHash::Keccak_256,
        KECCAK_512 = QCryptographicHash::Keccak_512,
    };

    explicit QCKHash(HashType ht);
    ~QCKHash();

    void reset();

    void addData(const char *data, int length);
    void addData(const QByteArray &data);

    QByteArray result() const;
    const QString name() const{
        return _name;
    }

};

#endif // QCKHASH_H
