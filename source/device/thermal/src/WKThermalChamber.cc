#include "WKThermalChamber.hh"

#include <iostream>
#include <thread>
#include <chrono>

#include <unistd.h>

void WKThermalChamber::Init(){

  const std::string mx_path = "/dev/motory";
  const std::string my_path = "/dev/motorx";
  const std::string ir_path = "/dev/ircamera";

  if(!access(mx_path.c_str(), R_OK|W_OK)){
    m_mx.reset(new nanotec::NanotecMotor(mx_path));
    m_mx->setTravelDistance(0);
    int posx = m_mx->getTravelDistance();
    m_mx->setLimitSwitchBehaviour(2,8,2,2);//ext_ref_back
    m_mx->setMaximumFrequency(200);
    m_mx->setDirection(0);
    m_mx->setRampType(2);
    if(!m_mx->motorIsReferenced()){
      m_mx->setPositionMode(4);//ext_ref
      m_mx->setTravelDistance(0);
      posx = m_mx->getTravelDistance();
      m_mx->startMotor();
    }
    else{
      m_mx->startMotor();
    }
    while(!m_mx->isStatusReady()){
      if(m_mx->isStatusErrorPos()){
	m_mx->resetPositionError();
	m_mx->setTravelDistance(posx);
	m_mx->startMotor();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }
  
  if(!access(my_path.c_str(), R_OK|W_OK)){
    m_my.reset(new nanotec::NanotecMotor(my_path));
    m_my->setTravelDistance(0);
    int posy = m_my->getTravelDistance();
    m_my->setLimitSwitchBehaviour(2,8,1,2);//ext_ref_fwd
    m_my->setPositionMode(4);//ext_ref
    m_my->setDirection(1);
    m_my->setRampType(2);
    if(!m_my->motorIsReferenced()){
      m_my->setPositionMode(4);//ext_ref
      m_my->setTravelDistance(0);
      posy = m_my->getTravelDistance();
      m_my->startMotor();
    }
    else{
      m_my->startMotor();
    }
    while(!m_my->isStatusReady()){
      if(m_my->isStatusErrorPos()){
	m_my->resetPositionError();
	m_my->setTravelDistance(posy);
	m_my->startMotor();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }
  
  if(!access(ir_path.c_str(), R_OK|W_OK)){
    m_ir.reset(new CameraIR(ir_path));
  }
}


void WKThermalChamber::MoveToPositionX(int x){
  m_mx->setTravelDistance(x);
  m_mx->startMotor();
  while(!m_mx->isStatusReady()){
    if(m_mx->isStatusErrorPos()){
      m_mx->resetPositionError();
      m_mx->setTravelDistance(x);
      m_mx->startMotor();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}


void WKThermalChamber::MoveToPositionY(int y){
  m_my->setTravelDistance(y);
  m_my->startMotor();
  while(!m_my->isStatusReady()){
    if(m_my->isStatusErrorPos()){
      m_my->resetPositionError();
      m_my->setTravelDistance(y);
      m_my->startMotor();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

void WKThermalChamber::MoveToPosition(int x, int y){
  MoveToPositionX(x);
  MoveToPositionY(y);
}


void WKThermalChamber::TakePhoto(){
  m_ir->TakePhoto();
}
