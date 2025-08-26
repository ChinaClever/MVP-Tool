#include "test_fabpartition.h"

Test_Fabpartition::Test_Fabpartition(QObject *parent):BaseThread(parent)
{

}

Test_Fabpartition *Test_Fabpartition::build(QObject *parent)
{
    static Test_Fabpartition *sington = nullptr;
    if(sington == nullptr)
            sington = new Test_Fabpartition(parent);
    return sington;
}

bool Test_Fabpartition::programFull()
{

}

bool Test_Fabpartition::check()
{
    bool ret = at91recovery();
    if(ret) ret = devExist();
    return ret;
}

bool Test_Fabpartition::at91recovery()
{
    QString fn = mDir + "at91recovery";
    bool ret = isFileExist(fn);
    if(ret){
       //processOn()
    }
    else{

    }
    return ret;
}

bool Test_Fabpartition::devExist()
{
    bool ret;
    return ret;
}

bool Test_Fabpartition::isFileExist(const QString &fn)
{
    QFile file(fn);
    if (file.exists()){
        return true;
    }
    return false;
}

QString Test_Fabpartition::processOn(const QString &cmd)
{
    static char res[10][512];
    QString str;

#if defined(Q_OS_LINUX)
        QProcess process;
        //process.start("/bin/"
#else

#endif

}
