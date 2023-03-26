#include "widget.h"
#include "./ui_widget.h"


void Widget::on_textScheduledCheckNetTime_editingFinished()
{
    if (ui->textScheduledCheckNetTime->text().toInt() < 3)
        ui->textScheduledCheckNetTime->setText(QString("3"));
}

void Widget::dealScheduledCheckNetTunnelReturned(int m_currentTunnel)
{
    if (scheduledLoginStyle == 2 and m_currentTunnel != defaultTunnel)  // 第 3 种模式，非默认通道即登陆
        naManager->setTunnel(defaultTunnel);
    else if (m_currentTunnel > 8)                                       // 否则校内时才操作
    {
        if (scheduledLoginStyle == 1)                                   // 登陆默认通道
            naManager->setTunnel(defaultTunnel);
        else
            naManager->setTunnel(m_currentTunnel-1958);                 // 登陆当前通道
    }
}

void Widget::dealScheduledCheckNetTimerTimeout()
{
    if (ui->checkBoxScheduledTimeRange->isChecked())
    {
        if (vCheckboxWeek.at(currentDate->currentDate().dayOfWeek()-1)->isChecked()
                and currentTime->currentTime() > ui->timeStartTask->time()
                and currentTime->currentTime() < ui->timeEndTask->time())
            naManager->checkNet();
    }
    else
        naManager->checkNet();
}

void Widget::setCheckedTunnel(int checkedTunnel)
{
    for (int j = 0; j < 9; j++)
    {
        if (j == checkedTunnel)
            vActionTunnel.at(j)->setChecked(true);
        else
            vActionTunnel.at(j)->setChecked(false);
    }
}

void Widget::on_checkboxEnableScheduledCheckNet_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
    {
        ui->textScheduledCheckNetTime->setEnabled(true);
        ui->checkboxEnableScheduledLogin->setEnabled(true);
        ui->checkBoxScheduledTimeRange->setEnabled(true);
        ui->checkBoxIgnoreWarning->setEnabled(true);
    }
    else
    {
        ui->textScheduledCheckNetTime->setEnabled(false);
        ui->checkboxEnableScheduledLogin->setEnabled(false);
        ui->checkboxEnableScheduledLogin->setChecked(false);
        ui->checkBoxScheduledTimeRange->setEnabled(false);
        ui->checkBoxScheduledTimeRange->setChecked(false);
        ui->checkBoxIgnoreWarning->setEnabled(false);
    }
}

void Widget::on_checkboxEnableScheduledLogin_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
    {
        ui->comboboxScheduledLoginStyle->setEnabled(true);
    }
    else
    {
        ui->comboboxScheduledLoginStyle->setEnabled(false);
    }
}

void Widget::on_checkBoxScheduledTimeRange_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
    {
        ui->groupboxTimeRange->setEnabled(true);
    }
    else
    {
        ui->groupboxTimeRange->setEnabled(false);
    }
}

void Widget::on_timeStartTask_userTimeChanged(const QTime &time)
{
    if (ui->timeEndTask->time() < time)
    {
        ui->timeEndTask->setTime(time);
        ui->timeEndTask->setTimeRange(time,ui->timeEndTask->maximumTime());
    }
}

void Widget::on_checkboxEnableRunAtStartup_stateChanged(int checkState)
{
    if (checkState == Qt::Checked)
        this->setRunAtStartup(true);
    else
        this->setRunAtStartup(false);

}
