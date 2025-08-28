#ifndef TEST_FABPARTITION_H
#define TEST_FABPARTITION_H
#include <QObject>
#include "baselogs.h"

class Test_Fabpartition : public BaseThread
{
    Q_OBJECT
public:
    explicit Test_Fabpartition(QObject *parent = nullptr);
    static Test_Fabpartition *build(QObject *parent = nullptr);

    bool check();
    bool programFull();
    bool workDown();
    void secure_boot_prov();

protected:
    bool at91recovery();
    bool mvFile(bool res);
    bool programFab();
    bool readOutput(QProcess &pro);
    bool changePermissions();
    bool devExist();
    bool createFab();
    bool isFileExist(const QString &fn);
    int shexec(const char *cmd, char res[][512], int count);
    QString processOn(const QString &cmd);

signals:
    void fabSig(QString str);

private:
    QString mDir;
};

#endif // TEST_FABPARTITION_H
