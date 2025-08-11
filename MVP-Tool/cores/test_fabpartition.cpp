/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "test_fabpartition.h"

test_FabPartition::test_FabPartition(QObject *parent) : BaseThread(parent)
{
    mSn = Test_SerialNumber::bulid(this);
    mDir = "./Firmware_Build/4.0.0.5-48035/";
}

test_FabPartition *test_FabPartition::bulid(QObject *parent)
{
    static test_FabPartition* sington = nullptr;
    if(sington == nullptr)
        sington = new test_FabPartition(parent);
    return sington;
}

bool test_FabPartition::isFileExist(const QString &fn)
{
    QFile file(fn);
    if (file.exists()){
        return true;
    }

    return false;
}

int test_FabPartition::shexec(const char *cmd, char res[][512], int count)
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

QString test_FabPartition::processOn(const QString &cmd)
{
    static char res[10][512];

    QString str;
#if defined(Q_OS_LINUX)
    emit fabSig("shexec, cmd: －－－－－－－－－－－－－－－－\n" + cmd);
    char *ptr = cmd.toLatin1().data();
    int cnt = shexec(ptr, res, 10);
    for(int i=0; i<cnt; ++i) if(strlen(res[i])>2) str.append(res[i]);
    emit fabSig("return results: －－－－－－－－－－－－－－－－\n" +str);
#else
    updatePro(tr("不支持Window系统"), false);
#endif
    return str;
}

bool test_FabPartition::devExist()
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

void test_FabPartition::secure_boot_prov()
{
    QString cmd = "cd " + mDir +"secure_boot_prov-scalepoint-040000-48035/ \n"
                  "echo \"123456\" | sudo -S sh secure_boot_permanent_scalepoint.sh";
    processOn(cmd.toLocal8Bit().data());
    updatePro(tr("启用完全引导"));
}

bool test_FabPartition::at91recovery()
{
    QString fn = mDir + "at91recovery";
    bool ret = isFileExist(fn);
    if(ret) {
        processOn("echo \"123456\" | sudo -S chmod 777 -R Firmware_Build/*");
    } else {
        updatePro(tr(" at91recovery 执行程序未发现"), ret);
    }

    return ret;
}

bool test_FabPartition::changePermissions()
{
    QString str = tr("改变IMG文件的权限");
    updatePro(tr("准备")+str);

    QString cmd = "echo \"123456\" | sudo -S chmod 777 -R " + mDir +
                  "*.img *.bin \n sudo chmod 777 /etc/pki/secure_boot_prov/*";
    processOn(cmd.arg(mDt->sn));
    return updatePro(tr("已")+str);
}

bool test_FabPartition::programFab()
{
    QString str = tr("写入S/N Mac ");
    updatePro(tr("准备")+str);

    QStringList ls;
    QProcess pro(this);
    ls << "-y" << "/dev/ttyACM0" << mDir + mDt->sn+".img" << "fab";
    pro.start(mDir +"at91recovery", ls);

    str = "S/N:" + mDt->sn + "  Mac:" + mItem->macs.mac;
    bool ret = readOutput(pro);
    if(ret) {
        str += tr(" 写入成功");
    } else {
        str += tr(" 写入失败");
    }

    mvFile(ret);
    return updatePro(str, ret);
}

bool test_FabPartition::createFab()
{
    QString cmd = "mkdir -p %1ScalePoint \n cd %1ScalePoint \n"
                  "rm -f system.cfg \n"
                  "echo \"MAC=%2\" > system.cfg \n"
                  "echo \"BOARD_SERIAL=%3\" >> system.cfg \n"
                  "cat system.cfg \n cd ../ \n"
                  "mkfs.cramfs -b 4096 ScalePoint/ %3.img \n";

    QString str = "create FAB partition ";
    bool ret = isFileExist(mDir +"ScalePoint/eto-desc.xml");
    if(ret) {
        QString res = processOn(cmd.arg(mDir).arg(mItem->macs.mac).arg(mDt->sn));
    } else {
        str = tr("配置文件缺少 eto-desc.xml");
    }

    return updatePro(str, ret);
}

bool test_FabPartition::createRaritanFab()
{
    QString cmd = "mkdir -p %1Raritan \n cd %1Raritan \n"
                  "rm -f system.cfg \n"
                  "echo \"MAC=%2\" > system.cfg \n"
                  "echo \"BOARD_SERIAL=%3\" >> system.cfg \n"
                  "cat system.cfg \n cd ../ \n"
                  "mkfs.cramfs -b 4096 Raritan/ %3.img \n";

    QString str = "create FAB partition ";
    bool ret = isFileExist(mDir +"Raritan/eto-desc.xml");
    if(ret) {
        QString res = processOn(cmd.arg(mDir).arg(mItem->macs.mac).arg(mDt->sn));
    } else {
        str = tr("配置文件缺少 eto-desc.xml");
    }

    return updatePro(str, ret);
}


bool test_FabPartition::mvFile(bool res)
{
    QString cmd = "cd %2 \n mkdir -p fabs fabs/%1 \n"
                  "mv ScalePoint/system.cfg fabs/%1/ \n"
                  "mv %1.img fabs/%1" ;
    if(res) {
        processOn(cmd.arg(mDt->sn).arg(mDir));
    } else {
        mItem->currentNum--;
        mSn->updateMacAddr(-1);
        Cfg::bulid()->setCurrentNum();
    }

    return res;
}

bool test_FabPartition::readOutput(QProcess &pro)
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


bool test_FabPartition::programFull()
{
    QStringList ls;
    QProcess pro(this);
    updatePro(tr("开始烧录镜像文件，请耐心等待"));

    if(mDt->img.contains(".img")) ls << "-y" << "/dev/ttyACM0" << mDt->img << "0x00000";
    else ls << "-y" << "/dev/ttyACM0" << mDt->img;
    QString exe = mDir + "at91recovery";
    pro.start(exe, ls);

    bool ret = readOutput(pro);
    QString str = tr("录镜像文件，写入");
    if(ret) str += tr("成功"); else str += tr("失败");
    return updatePro(str, ret);
}

bool test_FabPartition::check()
{
    bool ret = at91recovery();
    if(ret) ret = devExist();

    return ret;
}

bool test_FabPartition::workDown(int mode)
{
    bool ret = check();
    if(mode == 1){
        if(ret) ret = createFab();
    }else if(mode == 2){
        if(ret) ret = createRaritanFab();
    }
//    mDt->sn = "2I3B200002";
//    mItem->macs.mac = "00:04:74:1E:00:01";
    if(ret) ret = changePermissions();
    if(ret) ret = programFab();
    if(ret) secure_boot_prov();

    return ret;
}


