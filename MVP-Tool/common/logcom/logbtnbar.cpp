/*
 *
 *  Created on: 2020年10月1日
 *      Author: Lzy
 */
#include "logbtnbar.h"

LogBtnBar::LogBtnBar(QWidget *parent) : SqlBtnBar(parent)
{
    setNoEdit();
    //clearHidden();
    mQueryDlg = nullptr;
}


QString LogBtnBar::queryBtn()
{
    QString str;
    if(mQueryDlg) {
        int ret = mQueryDlg->Exec();
        if(ret == QDialog::Accepted) {
            str = mQueryDlg->getCmd();
        }
    }

    return str;
}
