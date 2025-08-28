#include "test_fabpartition.h"
#include <QtCore/qglobal.h>
Test_Fabpartition::Test_Fabpartition(QObject *parent):BaseThread(parent)
{

    mDir = "/home/ubuntu/MVP3/MVP3/MVP3_Projects/MVP3_FW/";
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
    QStringList ls;
    QProcess pro(this);
    updatePro(tr("开始烧录镜像文件，请耐心等待"));

    if(mIdGen->getImg().contains(".img")) ls << "-y" << "/dev/ttyACM0" << mIdGen->getImg() << "0x00000";
}

bool Test_Fabpartition::check()
{
    bool ret = at91recovery();

    if(ret) ret = devExist();
    return ret;
}

bool Test_Fabpartition::workDown()
{
    bool ret = check();
    if(ret)createFab();
//    if(ret)changePermissions();
//    if(ret) ret = programFab();

    return ret;
}
void Test_Fabpartition::secure_boot_prov()
{
    QString cmd = "cd " + mDir +"secure_boot_prov-scalepoint-040000-48035/ \n"
                  "echo \"123456\" | sudo -S sh secure_boot_permanent_scalepoint.sh";
    processOn(cmd.toLocal8Bit().data());
    updatePro(tr("启用完全引导"));
}

bool Test_Fabpartition::createFab()
{
    QString cmd =
        "mkdir -p %1ScalePoint \n cd %1ScalePoint \n"
        "rm -f system.cfg \n"
        "echo \"BOARD_SERIAL=%2\"  > system.cfg \n"
        "echo \"UNIT_SERIAL=%3\"  >> system.cfg \n"
        "echo \"MAC=%4\"          >> system.cfg \n"
        "echo \"MAC1=%5\"         >> system.cfg \n"
        "echo \"MAC2=%6\"         >> system.cfg \n"
        "echo \"MAC3=%7\"         >> system.cfg \n"
        "echo \"MAC4=%8\"         >> system.cfg \n"
        "echo \"ZIGBEE_MAC=%9\"   >> system.cfg \n"
        "echo \"BLUETOOTH_MAC=%10\" >> system.cfg \n"
        "cat system.cfg \n cd ../ \n"
        "mkfs.cramfs -b 4096 ScalePoint/ %3.img \n";

    QString str = "create FAB partition ";
    bool ret = isFileExist(mDir +"ScalePoint/eto-desc.xml");
    if(ret) {
        QString res = processOn(
            cmd.arg(mDir)
               .arg(mIdGen->mac.BOARD_SERIAL)         // BOARD_SERIAL
               .arg(mIdGen->getSN())     // UNIT_SERIAL
               .arg(mIdGen->mac.MAC)   // MAC
               .arg(mIdGen->mac.MAC1)  // MAC1
               .arg(mIdGen->mac.MAC2)  // MAC2
               .arg(mIdGen->mac.MAC3)  // MAC3
               .arg(mIdGen->mac.MAC4)  // MAC4
               .arg(mIdGen->mac.ZIGBEE_MAC)     // ZIGBEE_MAC
               .arg(mIdGen->mac.BLUETOOTH_MAC)  // BLUETOOTH_MAC
        );
    } else {
        str = tr("配置文件缺少 eto-desc.xml");
    }

    return updatePro(str, ret);
}

bool Test_Fabpartition::at91recovery()
{
    QString fn = mDir + "at91recovery";
    bool ret = isFileExist(fn);

    if(ret){
        // 只需要为 at91recovery 文件本身设置权限
        QString command = QString("echo \"123456\" | sudo -S chmod 777 \"%1\"").arg(fn);
        processOn(command);

        // 使用 updatePro 发送成功信号
        updatePro(tr("at91recovery 文件权限设置成功"), true);
    }
    else{
        // 使用 updatePro 发送失败信号
        updatePro(tr("at91recovery 执行程序未发现，请从 Firmware Build/ 复制到: %1").arg(fn), false);
    }

    return ret;
}

bool Test_Fabpartition::devExist()
{
    QString str = "Atmel USB";
    bool ret = isFileExist("/dev/ttyACM0");
    if(ret) {
        str += tr("已连接");
        processOn("echo \"123456\" | sudo -S chmod 777 /dev/ttyACM0");
    } else {
        str += tr("未找到设备，请确认烧录线是否连接正确？");
    }

    return updatePro(str, ret);
}

bool Test_Fabpartition::isFileExist(const QString &fn)
{
    QFile file(fn);
    if (file.exists()){
        return true;
    }
    return false;
}

int Test_Fabpartition::shexec(const char *cmd, char res[][512], int count)
{
    int i = 0;
#if defined(Q_OS_LINUX)
    FILE* pp = popen(cmd, "r");
    if(!pp) {
        qDebug("error, cannot popen cmd: %s\n", cmd);
        return -1;
    }

    res[0][0] = 0;
    char tmp[512] ={0};
    while(fgets(tmp, sizeof(tmp), pp) != NULL) {
        if(tmp[strlen(tmp)-1] == '\n') {
            //tmp[strlen(tmp)-1] = '\0';
        }
        // qDebug("%d.get return results: %s\n", i, tmp);
        strcpy(res[i], tmp); i++;
        if(i >= count) {
            qDebug("get enough results, return\n");
            break;
        }
    }

    int rv = pclose(pp);
    // qDebug("ifexited: %d\n", WIFEXITED(rv));
    if (WIFEXITED(rv)) {
        qDebug("subprocess exited, exit code: %d\n", WEXITSTATUS(rv));
    }
#endif

    return i;
}

bool Test_Fabpartition::changePermissions()
{
    QString str = tr("改变IMG文件的权限");
    updatePro(tr("准备")+str);

    QString cmd = "echo \"123456\" | sudo -S chmod 777 -R " + mDir +
                  "*.img *.bin \n sudo chmod 777 /etc/pki/secure_boot_prov/*";
    processOn(cmd);
    return updatePro(tr("已")+str);
}

bool Test_Fabpartition::mvFile(bool res)
{
    QString cmd = "cd %2 \n mkdir -p fabs fabs/%1 \n"
                  "mv ScalePoint/system.cfg fabs/%1/ \n"
                  "mv %1.img fabs/%1" ;
    if(res) {
        processOn(cmd.arg(mIdGen->getSN()).arg(mDir));
    } else {

        Cfg::bulid()->setCurrentNum();
    }

    return res;
}

bool Test_Fabpartition::readOutput(QProcess &pro)
{
    bool ret, res = true;
    do {
        ret = pro.waitForFinished(1000);
        QByteArray bs = pro.readAllStandardOutput();
        bs +=  pro.readAllStandardError();
        QString str = QString::fromLocal8Bit(bs);
        if(str.contains("ERR")) res = false; //else str = str.simplified();
        if(str.size() > 2) emit fabSig(str);
    } while(!ret);

    pro.close();
    return res;
}

bool Test_Fabpartition::programFab()
{
    QString str = tr("写入S/N Mac ");
    updatePro(tr("准备")+str);

    QStringList ls;
    QProcess pro(this);
    ls << "-y" << "/dev/ttyACM0" << mDir + mIdGen->getSN()+".img" << "fab";
    pro.start(mDir +"at91recovery", ls);

    str = "S/N:" + mIdGen->getSN();
    bool ret = readOutput(pro);
    if(ret) {
        str += tr(" 写入成功");
    } else {
        str += tr(" 写入失败");
    }

    mvFile(ret);
    return updatePro(str, ret);
}

QString Test_Fabpartition::processOn(const QString &cmd)
{
    static char res[10][512];
    QString str;

#if defined(Q_OS_LINUX)
        QProcess process;
        emit fabSig("shexec, cmd: -------------\n" + cmd);
        char *ptr = cmd.toLatin1().data();
        int cnt = shexec(ptr, res, 10);
        for(int  i = 0; i < cnt; i++)if(strlen(res[i])>2) str.append(res[i]);
        emit fabSig("return results: -------------\n" + str);
#else
        updatePro(tr("不支持Window系统",false);
#endif
        return str;

}
