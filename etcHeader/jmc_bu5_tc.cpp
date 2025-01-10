#include "jmc_axis_cmd.h"






//class JMC_Reset : public JMC_Axis_Cmd{
//    void call(QString axis_name,bool xExecute);
//    void call(axis_ref *axis,bool xExecute);
//private:
//};

int JMC_BU5_TC::setParam(axis_ref *axis,double vel,double acc,double dec,double jerk,double end_vel,double pressure,int conf)
{

    if (acc <= 0){
        return 1;
    }
    if (dec <= 0){
        return 2;
    }
    if (vel <= 0){
        return 3;
    }
    if(jerk <= 0){
        return 4;
    }
    if(pressure <= 0){
        return 5;
    }
    if (!ec::axis_can_bu5_tc(axis)){
        return 10;
    }

    axis->cmd.bu5_tc.vel = vel;
    axis->cmd.bu5_tc.acc = acc;
    axis->cmd.bu5_tc.dec = dec;
    axis->cmd.bu5_tc.jerk = jerk;
    axis->cmd.bu5_tc.end_vel = end_vel;
    axis->cmd.bu5_tc.pressure = pressure;
    axis->cmd.bu5_tc.conf = conf;
    return 0;
}
void JMC_BU5_TC::copyParam(bool xExecute,double vel,double acc,double dec,double jerk,double end_vel,double pressure,int conf)
{
    Execute = xExecute;
    fVel = vel;
    fAcc = acc;
    fDec = dec;
    fJerk = jerk;
    fEnd_Vel = end_vel;
    fPressure = pressure;
    iConf = conf;
}
void JMC_BU5_TC::call(axis_ref *axis)
{
    call(axis, Execute, fVel, fAcc, fDec, fJerk, fEnd_Vel, fPressure, iConf);
}
void JMC_BU5_TC::call(QString axis_name)
{
    call(axis_name, Execute, fVel, fAcc, fDec, fJerk, fEnd_Vel, fPressure, iConf);
}
void JMC_BU5_TC::call(QString axis_name,bool xExecute,double vel,double acc,double dec,double jerk,double end_vel,double pressure,int conf)
{
    axis_ref *axis = ec::get_axis(axis_name);
    if (axis == nullptr){
        out_error(2);
        return;
    }
    call(axis,xExecute,vel, acc, dec,jerk, end_vel,pressure,conf);
}
void JMC_BU5_TC::call(axis_ref *axis,bool xExecute,double vel,double acc,double dec,double jerk,double end_vel,double pressure,int conf)
{
    copyParam( xExecute, vel, acc, dec, jerk, end_vel, pressure, conf);
    if(xExecute && not oldExecute){
        state = 4;

    }
    switch (state) {
        case 0:
            tcState = 0;
            out_clear();
        break;
        case 4:
            if (ec::axis_can_bu5_tc(axis)){
                state = 5;
            }else{
                state = 15;
            }
        break;
        case 5:{
            int ret = setParam(axis, vel, acc, dec, jerk, end_vel, pressure,conf);
            if (ret != 0){
                state = 26;
                break;
            }
            ret = ec::get_new_act(axis,cmd_type::bu5_tc,&actNo);
            if (ret != 0){
                state = 26;
                break;
            }
            axis->cmd.bu5_tc.done = false;
            axis->cmd.bu5_tc.error = false;
            axis->cmd.bu5_tc.errorid = 0;
            state = 10;

        }

        break;
        case 10:
            if (ec::axis_in_act(axis,actNo)){
                axis->cmd.bu5_tc.keep_enbale =  xAdj;
                tcState = axis->cmd.bu5_tc.state;
                if (axis->cmd.bu5_tc.done){
                    tcState = 0;
                    state = 15;
                }
                if (axis->cmd.bu5_tc.error){
                    tcState = 0;
                    state = 25;
                }
            }else{
                state = 20;
            }
        break;
        case 15:
            out_done();
            if(!xExecute){
                state = 0;
            }
        break;
        case 20:
            out_abort();
            if(!xExecute){
                state = 0;
            }
        break;
        case 25:
            out_error(axis->cmd.errorID);
            if(!xExecute){
                state = 0;
            }
        break;
        case 26:
            out_error(1);
            if(!xExecute){
                state = 0;
            }
        break;
        default:

        break;
    }



    oldExecute = xExecute;
}
