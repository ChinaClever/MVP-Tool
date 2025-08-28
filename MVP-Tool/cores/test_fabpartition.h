#ifndef TEST_FABPARTITION_H
#define TEST_FABPARTITION_H
#include "baseobject.h"
#include <QObject>
class Test_Fabpartition : public BaseThread
{
    Q_OBJECT
public:
    explicit Test_Fabpartition(QObject *parent = nullptr);
    static Test_Fabpartition *build(QObject *parent = nullptr);

    bool check();
    bool programFull();
    bool workDown();
    bool createFab();
    bool changePermissions();
    void secure_boot_prov();
    bool programFab();
    bool readOutput(QProcess &pro);
    bool mvFile(bool);
protected:
    bool at91recovery();
    bool devExist();
    bool isFileExist(const QString &fn);
    QString processOn(const QString &cmd);

signals:
    void fabSig(QString str);

private:
    QString mDir;
};

#endif // TEST_FABPARTITION_H
