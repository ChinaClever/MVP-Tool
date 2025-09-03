#include "test_fabpartition.h"

Test_Fabpartition::Test_Fabpartition(QObject *parent):BaseThread(parent)
{
    mDir = "./Firmware_Build/4.0.3.5-51776/";
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
    qDebug()<<fn;
    bool ret = isFileExist(fn);
    if(ret) {
        processOn("echo \"123456\" | sudo -S chmod +x " + fn); //？？？
    } else {
        updatePro(tr(" at91recovery 执行程序未发现"), ret);
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

void Test_Fabpartition::secure_boot_prov()
{
    QString cmd = "cd " + mDir +"secure_boot_prov-scalepoint-040350-51776/ \n"
                                 "echo \"123456\" | sudo -S sh secure_boot_permanent_scalepoint.sh";
    processOn(cmd.toLocal8Bit().data());
    updatePro(tr("启用完全引导"));
}


bool Test_Fabpartition::workDown()
{
    bool ret = check();


    //=====================
//    if(ret) enterBootloaderMode();
//    if(ret) programMainFirmware();
 //   if(ret) enterBootloaderMode();
    //=====================

     if(ret) ret = createFab();


     if(ret) ret = changePermissions();
     if(ret) ret = programFab();
     if(ret) secure_boot_prov();
     // 只有烧录成功才保存 MAC
     if(ret) {
         DeviceIdGenerator::instance().saveMacs();
         emit renewMacSig();
     }

    return ret;
}

bool Test_Fabpartition::enterBootloaderMode()
{
    // 提示用户进行硬件操作
    updatePro(tr("请执行以下硬件操作：\n"
                 "1. 关闭MVP3电源\n"
                 "2. 短接J8跳线\n"
                 "3. 连接USB线\n"
                 "4. 给MVP3上电\n"
                 "5. 等待5秒后断开J8跳线\n"
                 "等待设备识别..."));
sleep(10);
    // 等待设备出现
    for(int i = 0; i < 30; i++) { // 等待30秒
        QThread::sleep(1);
        if(isFileExist("/dev/ttyACM0")) {
            processOn("echo \"123456\" | sudo -S chmod 777 /dev/ttyACM0");
            return updatePro(tr("设备已进入Bootloader模式"), true);
        }
    }

    return updatePro(tr("设备进入Bootloader模式超时，请检查硬件连接"), false);
}


bool Test_Fabpartition::programMainFirmware()
{
    updatePro(tr("准备烧录主固件"));

    QStringList ls;
    ls << "-y" << "/dev/ttyACM0" << mDir + "aggregator-ixg4_64-040350-51776.bin";

    QProcess pro;
    pro.start(mDir + "at91recovery", ls);

//    if(!pro.waitForStarted(5000) || !pro.waitForFinished(120000)) {
//        return updatePro(tr("主固件烧录超时"), false);
//    }

    bool ret = readOutput(pro);
    return updatePro(ret ? tr("主固件烧录成功") : tr("主固件烧录失败"), ret);
}

bool Test_Fabpartition::changePermissions()
{
    QString str = tr("改变IMG文件的权限");
    updatePro(tr("准备")+str);

//    QString cmd = "echo \"123456\" | sudo -S chmod 777 -R " + mDir +
//                  "*.img *.bin \n sudo chmod 777 /etc/pki/secure_boot_prov/*";

//    processOn(cmd.arg(mIdGen->getSN()));
//    return updatePro(tr("已")+str);

    QString cmd = QString(
        "echo \"123456\" | sudo -S chmod 777 -R %1*.{img,bin} 2>/dev/null\n"
        "sudo chmod 777 /etc/pki/secure_boot_prov/* 2>/dev/null")
        .arg(mDir);                          // <-- PATCH
    processOn(cmd);                         // <-- PATCH
    return updatePro(tr("已") + str);

}

bool Test_Fabpartition::createFab()
{
    // 检查 XML 配置是否存在
    bool ret = isFileExist(mDir + "ScalePoint/eto-desc.xml");
    if (!ret) {
        updatePro(tr("配置文件缺少 eto-desc.xml"), false);
        return false;
    }

    // 获取一批 MAC
    QString boardSN = mIdGen->getSN();
    auto macBatch = mIdGen->allocateBatch();
    const QList<QString>& macs = macBatch["mac"];
    const QList<QString>& zigbees = macBatch["zigbee"];

    // 安全取值，防止数组越界
    QString mac0 = macs.value(0, "");
    QString mac1 = macs.value(1, "");
    QString mac2 = macs.value(2, "");
    QString mac3 = macs.value(3, "");
    QString mac4 = macs.value(4, "");
    QString bluetoothMac = macs.value(5, ""); // 蓝牙取最后一个
    QString zigbeeMac = zigbees.value(0, "");

    // 删除旧的 img 文件
    QString cleanCmd = QString("cd %1ScalePoint && rm -f *.img").arg(mDir);
    processOn(cleanCmd);

    // 拼接命令生成新的 img
    QString cmd = QString(
                      "mkdir -p %1ScalePoint\n"
                      "cd %1ScalePoint\n"
                      "rm -f system.cfg\n"
                      "echo \"BOARD_SERIAL=%2\"  > system.cfg\n"
                      "echo \"UNIT_SERIAL=%3\"   >> system.cfg\n"
                      "echo \"MAC=%4\"           >> system.cfg\n"
                      "echo \"MAC1=%5\"          >> system.cfg\n"
                      "echo \"MAC2=%6\"          >> system.cfg\n"
                      "echo \"MAC3=%7\"          >> system.cfg\n"
                      "echo \"MAC4=%8\"          >> system.cfg\n"
                      "echo \"ZIGBEE_MAC=%9\"    >> system.cfg\n"
                      "echo \"BLUETOOTH_MAC=%10\" >> system.cfg\n"
                      "cat system.cfg\n"
                      "cd ../\n"
                      "mkfs.cramfs -b 4096 ScalePoint/ %2.img\n"
                      ).arg(mDir)
                      .arg(boardSN)
                      .arg(mDt->unitSn)
                      .arg(mac0)
                      .arg(mac1)
                      .arg(mac2)
                      .arg(mac3)
                      .arg(mac4)
                      .arg(zigbeeMac)
                      .arg(bluetoothMac);

    // 执行命令
    QString res = processOn(cmd);

    return updatePro(tr("create FAB partition"), true);
}

bool Test_Fabpartition::programFab()
{
    updatePro(tr("准备写入 S/N 和 MAC 地址"));

    // 获取批量 MAC
    auto macBatch = mIdGen->allocateBatch();
    const QList<QString>& macs = macBatch["mac"];
    const QList<QString>& zigbees = macBatch["zigbee"];
    QString boardSN = mIdGen->getSN();

    QString mac0 = macs.value(0, "");
    QString mac1 = macs.value(1, "");
    QString mac2 = macs.value(2, "");
    QString mac3 = macs.value(3, "");
    QString mac4 = macs.value(4, "");
    QString bluetoothMac = macs.value(5, "");
    QString zigbeeMac = zigbees.value(0, "");

    QStringList ls;
    QProcess pro;
    ls << "-y" << "/dev/ttyACM0" << mDir + boardSN + ".img" << "fab";

    pro.start(mDir + "at91recovery", ls);

    bool ret = readOutput(pro);

    // 生成日志
    QString logStr = QString("S/N:%1\nMAC0~4:%2,\n%3,\n%4,\n%5,\n%6\n蓝牙 MAC:%7\nZigbee MAC:%8")
                         .arg(boardSN)
                         .arg(mac0).arg(mac1).arg(mac2).arg(mac3).arg(mac4)
                         .arg(bluetoothMac)
                         .arg(zigbeeMac);
    logStr += ret ? "\n写入成功" : "\n写入失败";

    mvFile(ret); // 烧录成功后将cfg文件和镜像保存到文件夹fabs里
    return updatePro(logStr, ret);
}

bool Test_Fabpartition::mvFile(bool res)
{
    // 构建命令，把 system.cfg 和 img 文件移动到 fabs/序列号 目录下
    QString boardSN = mIdGen->getSN();
    QString cmd = QString(
                      "cd %2\n"
                      "mkdir -p fabs fabs/%1\n"
                      "mv ScalePoint/system.cfg fabs/%1/\n"
                      "mv %1.img fabs/%1/"
                      ).arg(boardSN)   // %1 -> 序列号
                      .arg(mDir);     // %2 -> 基础目录

    if(res) {
        // 烧录成功，就执行移动命令
        processOn(cmd);
    }
    return res;
}

bool Test_Fabpartition::readOutput(QProcess &pro)
{
    bool ret, res = true;
    do {
        ret = pro.waitForFinished(5000);
        QByteArray bs = pro.readAllStandardOutput();
        bs +=  pro.readAllStandardError();
        QString str = QString::fromLocal8Bit(bs);
        qDebug()<<"str:::"<<bs;
        if(str.contains("ERR")) res = false; //else str = str.simplified();
        if(str.size() > 2) emit fabSig(str);
    } while(!ret);

    pro.close();
    return res;
}


bool Test_Fabpartition::isFileExist(const QString &fn)
{
    QFile file(fn);
    bool exists = file.exists();

    if (!exists) {
        qDebug() << "错误信息:" << file.errorString();
    }

    return exists;
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
QString Test_Fabpartition::processOn(const QString &cmd)
{
    static char res[10][512];
    QString str;

#if defined(Q_OS_LINUX)
    emit fabSig("shexec,cmd:--------------\n" + cmd);
    char *ptr = cmd.toLatin1().data();
    int cnt = shexec(ptr,res,10);
    for(int i = 0; i < cnt; i++)if(strlen(res[i])>2)str.append(res[i]);
    emit fabSig("return results: -----------------\n"+str);
    QProcess process;

#else
    updatePro(tr("不支持Window系统"),false);
#endif
    return str;

}
