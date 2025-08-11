#include "datapacket.h"

sDataPacket::sDataPacket()
{
    dev = new sDevData;
    pro = new sProgress();
}

sDataPacket *sDataPacket::build()
{
    static sDataPacket* sington = nullptr;
    if(sington == nullptr)
        sington = new sDataPacket();
    return sington;
}

void sDataPacket::init()
{
    pro->step = 0;
    pro->result = 0;
    pro->pass.clear();
    pro->status.clear();

    dev->dt.sn.clear();              // 清空序列号
    dev->dt.hwVersion.clear();       // 清空硬件版本
    dev->dt.fwVersion.clear();       // 清空固件版本
    dev->dt.eth1Mac.clear();         // 清空 ETH1 MAC 地址
    dev->dt.eth2Mac.clear();         // 清空 ETH2 MAC 地址
    dev->dt.eth3Mac.clear();         // 清空 ETH3 MAC 地址
    dev->dt.spe1Mac.clear();         // 清空 SPE1 MAC 地址
    dev->dt.spe2Mac.clear();         // 清空 SPE2 MAC 地址
    dev->dt.btMac.clear();           // 清空 BT MAC 地址
    dev->dt.zbMac.clear();           // 清空 ZB MAC 地址
    dev->dt.pcbCode.clear();         // 清空 PCB 码
    dev->dt.state = 0;               // 清空状态
    dev->dt.reason.clear();          // 清空原因


    pro->result = Test_Info;
    pro->startTime = QTime::currentTime();
}
