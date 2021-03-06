/*! \file ****************************************************************************************************************************

  COMPANY:           Continental Automotive: Advanced Driver Assistance Systems - Advanced Engineering (for HackTM participants)

  PROJECT:           ...

  MODULNAME:         aero_hacktm_task.cpp

  DESCRIPTION:       HackTM module reading inputs from from Stereo Camera, Long Range Radar (both front center), Vehicle Dynamics
                     and GPS data.

  CREATION DATE:     28.03.2016

  VERSION:           $Revision: 1.1 $

  Hacked by Silviu-Marius Ardelean @ #HackTM2016
  http://silviuardelean.ro	
  https://twitter.com/silviuardelean

  -------------------------------------------------
  About ADAS Advanced Engineering:
  -------------------------------------------------
    http://tinyurl.com/ContiAdasAeInterview 
    http://tinyurl.com/ContiAutomatedDriving 
    http://tinyurl.com/ContiADAS 
	
*/

   /*********************************************************************************************************************************/
   // The example below reads specific input data provided by the sensors. Users will insert their code in the "wxEVT_STEP" function.
   // "wxEVT_STEP" is called cyclicly.
   //
   // Information provided:
   //---------------------
   //      - Traffic participants data from Stereo Camera (SCAM) Front Center (FC)    => SCAM_FC
   //      - Traffic participants data from Long Range Radar (LRR) Front Center (FC)  => LLR_FC
   //      - Vehicle dynamics data from a separate ECU                                => VEH_DYN_Data
   //      - GPS data                                                                 => GPS_Data
   /*********************************************************************************************************************************/

//////////////////////////////////////////////////////////
//           headers
//////////////////////////////////////////////////////////
#include "aero_hacktm_task.h"
#include "VehicleConfig.h"
#include "LogWriterModPlugin.h"
#include "cpprest/http_msg.h"
#include "cpprest/http_msg.h"

#pragma comment(lib, "cpprest110_1_1")

using namespace Tasks::AERO_HACK_TM_Task; // Tasks part mandatory for runtask
using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::client;

//////////////////////////////////////////////////////////
//           plugin header
//////////////////////////////////////////////////////////
//! Specific name of the plugin
std::string PLUGIN_NAME = "AERO_HackTM";    // mandatory/standard for runtask
//! Additional plugin description
std::string PLUGIN_DESC = "A task which integrates the HackTM user app inside a Runtask framework";    // mandatory/standard for runtask



//////////////////////////////////////////////////////////
//           class CModPlugIn definition
//////////////////////////////////////////////////////////

/**
   * @brief  Default constructor. 
    *
     * @param inst_name  Name of the plugin instance.
      * @param callback   Pointer to the central callback hook (for internal use).
      **/
CModPlugIn::CModPlugIn(const char* inst_name, wxPlg_IntFunT callback)
             : CEvtPlugIn(inst_name, callback)
{

  //! Use the 'DEC_EVT' macro to register the callbacks (please comment out unused events)
  DEC_EVT(OBJECTEVT_OPEN,        wxEVT_OPEN);
  DEC_EVT(OBJECTEVT_CLOSE,       wxEVT_CLOSE);
  DEC_EVT(OBJECTEVT_CONNECT,     wxEVT_CONNECT);
  DEC_EVT(OBJECTEVT_DISCONNECT,  wxEVT_DISCONNECT);
  DEC_EVT(OBJECTEVT_STEP,        wxEVT_STEP);
  DEC_EVT(OBJECTEVT_DRAW2D,      wxEVT_DRAW2D);
  DEC_EVT(OBJECTEVT_DRAW3D,      wxEVT_DRAW3D);
  DEC_EVT(OBJECTEVT_SIZE,        wxEVT_SIZE);

   eCAL::Initialize(0, NULL, "HackTM Task");
   memset(&LRR_FC_TrafParticList,0,sizeof(LRR_FC_TrafParticList)); 
   memset(&SCAM_FC_TrafParticList,0,sizeof(SCAM_FC_TrafParticList));

   m_pHTTPClient = std::unique_ptr<web::http::client::http_client>(new http_client(U("http://172.16.4.27:8000/payload/")));
   m_pRESTSender = std::unique_ptr<CRESTSender>(new CRESTSender(*m_pHTTPClient));
}


/**
   * @brief Destructor.
   **/
CModPlugIn::~CModPlugIn()
{
  eCAL::Finalize();

 // delete m_pHTTPClient;
}


//////////////////////////////////////////////////////////
//           class CModPlugIn events definition
//////////////////////////////////////////////////////////
//! Event OPEN will be called after host application has load the dll
long CModPlugIn::wxEVT_OPEN(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
   helpers::IniFile iniFile; arg_obj; ret_obj;

   /* Set up the log writer*/
   LogWriterModPlugin *logWriter = new LogWriterModPlugin(this);
   CLogDevice::get()->addLogWriter(logWriter);
   LOG_INFO("AERO_HackTM build %s %s", __DATE__, __TIME__);

   return(0);
};


//! Event CLOSE will be called before host application will unload the dll
long CModPlugIn::wxEVT_CLOSE(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
   TextPrint2Con("wxEVT_CLOSE was called"); arg_obj; ret_obj;
   return(0);
};

//! Event CONNECT will be called after host application has gone in connect state
// e.g. wxWavE's "Network Connect" or RunTask's "Start"
long CModPlugIn::wxEVT_CONNECT(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
  TextPrint2Con("wxEVT_CONNECT was called"); arg_obj; ret_obj;

  mSubTP_LRR_FC = new eCAL::CProtoSubscriber<class aero_trafpartic_protobuf::AERO_t_TrafParticList_Proto>(AERO_Proto::AERO_LRR_FC_TPLChannel);
  mSubTP_SCAM_FC = new eCAL::CProtoSubscriber<class aero_trafpartic_protobuf::AERO_t_TrafParticList_Proto>(AERO_Proto::AERO_SCAM_FC_TPLChannel);
  mSub_VEH_DYN_DATA = new eCAL::CProtoSubscriber<class aero_vehicle_data_protobuf::AERO_t_VehicleDynamics_Proto>(AERO_Proto::AERO_Vehicle_Dynamics_Channel);
  mSub_GPS_DATA = new eCAL::CProtoSubscriber<class aero_gps_rmc_protobuf::AERO_t_GpsRmc_Proto>(AERO_Proto::AERO_GPS_RMC_Data_Channel);

  mSubTP_LRR_FC->AddReceiveCallback(std::bind(&CModPlugIn::AEROTP_TrafParticReceive_LRR_FC,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  mSubTP_SCAM_FC->AddReceiveCallback(std::bind(&CModPlugIn::AEROTP_TrafParticReceive_SCAM_FC,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  mSub_VEH_DYN_DATA->AddReceiveCallback(std::bind(&CModPlugIn::AEROTP_DataReceive_VEH_DYN,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  mSub_GPS_DATA->AddReceiveCallback(std::bind(&CModPlugIn::AEROTP_DataReceive_GPS,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

   return(0);
};


//! Event DISCONNECT will be called before host application is going in disconnect state
// e.g. wxWavE's "Network Disconnect" or RunTask's "Stop"
long CModPlugIn::wxEVT_DISCONNECT(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
	 arg_obj; ret_obj;

  if (mSubTP_LRR_FC != NULL)     delete mSubTP_LRR_FC;
  if (mSubTP_SCAM_FC != NULL)    delete mSubTP_SCAM_FC;
  if (mSub_VEH_DYN_DATA != NULL) delete mSub_VEH_DYN_DATA;
  if (mSub_GPS_DATA != NULL)     delete mSub_GPS_DATA;
  	
  return(0);
};

//! Event STEP will be called every n-th ms depending from host application settings
long CModPlugIn::wxEVT_STEP(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
	 arg_obj; ret_obj;

   LOG_INFO("AERO: HackTM Step called:");

   /*********************************************************************************************************************************/
   // PLEASE INSERT THE BEST HACK_TM CODE HERE!
   /*********************************************************************************************************************************/
   
   auto jsonObject = json::value::object();

   LOG_INFO("VehDyn.Timestamp %ld", VEH_DYN_Data.SignalHeader.s_Timestamp);
   jsonObject[L"timestamp"] = (VEH_DYN_Data.SignalHeader.e_SignalStatus == AERO_SIGNAL_STATUS_OK) 
									? json::value::number((double)VEH_DYN_Data.SignalHeader.s_Timestamp)
									: json::value::number((double) time(NULL));

   jsonObject[L"vehicle"] = json::value::number(1);  // temporary apprach - VIN unavailable data

    LOG_INFO("VehDyn.Speed: %.1f", );
	jsonObject[L"speed"] = (VEH_DYN_Data.SpeedDisplayed.e_SignalStatus == AERO_SIGNAL_STATUS_OK) 
									? json::value::number(VEH_DYN_Data.SpeedDisplayed.f_Value)
									: json::value::number(0.0f);

   LOG_INFO("VehDyn.AccPedalPos: %.1f", VEH_DYN_Data.AccPedalPos.f_Value);
   jsonObject[L"acc_pedal_pos"] = (VEH_DYN_Data.AccPedalPos.e_SignalStatus == AERO_SIGNAL_STATUS_OK)  
									   ? json::value::number(VEH_DYN_Data.AccPedalPos.f_Value)
									   : json::value::number(0.0f);
   
    LOG_INFO("VehDyn.EngRPM: %.1f", VEH_DYN_Data.EngRPM.f_Value);
	jsonObject[L"rpm"] = (VEH_DYN_Data.EngRPM.e_SignalStatus == AERO_SIGNAL_STATUS_OK) 
								? json::value::number(VEH_DYN_Data.EngRPM.f_Value)
								: json::value::number(0.0f);
	
	LOG_INFO("VehDyn.Yawrate: %.1f", VEH_DYN_Data.Yawrate.f_Value);
	jsonObject[L"yawrate"] = (VEH_DYN_Data.Yawrate.e_SignalStatus == AERO_SIGNAL_STATUS_OK)
								? json::value::number(VEH_DYN_Data.Yawrate.f_Value)
								: json::value::number(0.0f);

   LOG_INFO("GPS_Data.f_Latitude: %f", RAD2DEG(GPS_Data.f_LatitudeRad));
   jsonObject[L"latitude"] = json::value::number(RAD2DEG(GPS_Data.f_LatitudeRad));
 
   LOG_INFO("GPS_Data.f_Longitude: %f", RAD2DEG(GPS_Data.f_LongitudeRad));
   jsonObject[L"longitude"] = json::value::number(RAD2DEG(GPS_Data.f_LongitudeRad));

   if (LRR_FC_TrafParticList.u_NumTrafficParticipants > 0)
   { 
	   json::value::element_vector	 arrayParticipants;

	   for (uint32 i = 0; i < LRR_FC_TrafParticList.u_NumTrafficParticipants; ++i)
	   {
		   json::value::field_map mapParticipants;
		   mapParticipants.push_back( make_pair(json::value(L"vel_x"), json::value::number(LRR_FC_TrafParticList.TrafPart[i].DynProp.Velocity.f_X)) );
		   mapParticipants.push_back( make_pair(json::value(L"vel_y"), json::value::number(LRR_FC_TrafParticList.TrafPart[i].DynProp.Velocity.f_Y)) );
		   mapParticipants.push_back( make_pair(json::value(L"length"), json::value::number(LRR_FC_TrafParticList.TrafPart[i].GeomProp.Length.f_Value)) );
		   mapParticipants.push_back( make_pair(json::value(L"height"), json::value::number(LRR_FC_TrafParticList.TrafPart[i].GeomProp.Height.f_Value)) );

		   LOG_INFO("LRR_FC_TrafParticList.TrafPart[0].DynProp.Velocity.f_X: %f", LRR_FC_TrafParticList.TrafPart[i].DynProp.Velocity.f_X);
           LOG_INFO("LRR_FC_TrafParticList.TrafPart[0].DynProp.Velocity.f_Y: %f", LRR_FC_TrafParticList.TrafPart[i].DynProp.Velocity.f_Y);
		   LOG_INFO("LRR_FC_TrafParticList.TrafPart[0].GeomProp.Length.f_Value: %f", LRR_FC_TrafParticList.TrafPart[i].GeomProp.Length.f_Value);
           LOG_INFO("LRR_FC_TrafParticList.TrafPart[0].GeomProp.Height.f_Value: %f", LRR_FC_TrafParticList.TrafPart[i].GeomProp.Height.f_Value);

		   arrayParticipants.push_back(std::make_pair(json::value::number(static_cast<int>(i)), json::value::object(mapParticipants)));
	   }

	   jsonObject[L"radar_participants"] = json::value::array(arrayParticipants);
	
   }
   
   if (SCAM_FC_TrafParticList.u_NumTrafficParticipants > 0)
   {
	   json::value::element_vector	 arrayParticipants;

	   for (uint32 i = 0; i < LRR_FC_TrafParticList.u_NumTrafficParticipants; ++i)
	   {
		   json::value::field_map mapParticipants;
		   mapParticipants.push_back( make_pair(json::value(L"vel_x"), json::value::number(SCAM_FC_TrafParticList.TrafPart[i].DynProp.Velocity.f_X)) );
		   mapParticipants.push_back( make_pair(json::value(L"vel_y"), json::value::number(SCAM_FC_TrafParticList.TrafPart[i].DynProp.Velocity.f_Y)) );
		   mapParticipants.push_back( make_pair(json::value(L"length"), json::value::number(SCAM_FC_TrafParticList.TrafPart[i].GeomProp.Length.f_Value)) );
		   mapParticipants.push_back( make_pair(json::value(L"height"), json::value::number(SCAM_FC_TrafParticList.TrafPart[i].GeomProp.Height.f_Value)) );

		   LOG_INFO("SCAM_FC_TrafParticList.TrafPart[0].GeomProp.Length: %.1f", SCAM_FC_TrafParticList.TrafPart[i].GeomProp.Length.f_Value);
		   LOG_INFO("SCAM_FC_TrafParticList.TrafPart[0].GeomProp.Height: %.1f", SCAM_FC_TrafParticList.TrafPart[i].GeomProp.Height.f_Value);
		   LOG_INFO("SCAM_FC_TrafParticList.TrafPart[0].GeomProp.Length.f_Value: %f", SCAM_FC_TrafParticList.TrafPart[i].GeomProp.Length.f_Value);
		   LOG_INFO("SCAM_FC_TrafParticList.TrafPart[0].GeomProp.Height.f_Value: %f", SCAM_FC_TrafParticList.TrafPart[i].GeomProp.Height.f_Value);

		   arrayParticipants.push_back(std::make_pair(json::value::number(static_cast<int>(i)), json::value::object(mapParticipants)));
	   }

	   jsonObject[L"camera_participants"] = json::value::array(arrayParticipants);
  }

   LOG_INFO("----------------------------------------------\n");

   m_pRESTSender->make_request(web::http::methods::POST, jsonObject);

   /*********************************************************************************************************************************/
   // End of DEMO code
   /*********************************************************************************************************************************/

   /* Return true to show output to hdc on the runtask, return false to ignore output*/
   return true;
};


//! Event DRAW2D will be called every n-th ms depending from host application settings
long CModPlugIn::wxEVT_DRAW2D(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
	arg_obj; ret_obj;    
  return(0);
}


//! Event DRAW3D will be called every n-th ms depending from host application settings
long CModPlugIn::wxEVT_DRAW3D(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
   LOG_INFO("Got triggered in DRAW3D ..."); arg_obj; ret_obj;

   // host app wxWavE
  if(GetHostAppName() == "wxWavE") { }

  // host app RunTask
  if(GetHostAppName() == "RunTask") { }
  
  return(0);
}


//! This will be called when a "change size" event of a drawing tab from the RunTask application
long CModPlugIn::wxEVT_SIZE(const wxPlg_Obj* arg_obj, wxPlg_Obj* ret_obj)
{
    arg_obj; ret_obj;
	return(0);
}


void CModPlugIn::AEROTP_TrafParticReceive_LRR_FC(const char* topic_name_, const class aero_trafpartic_protobuf::AERO_t_TrafParticList_Proto& msg_, const long long time_)
{
  AEROTrafparticProto_DeSerialize(&msg_,  &LRR_FC_TrafParticList); topic_name_; time_;
}


void CModPlugIn::AEROTP_TrafParticReceive_SCAM_FC(const char* topic_name_, const class aero_trafpartic_protobuf::AERO_t_TrafParticList_Proto& msg_, const long long time_)
{
  AEROTrafparticProto_DeSerialize(&msg_,  &SCAM_FC_TrafParticList); topic_name_; time_;
}


void CModPlugIn::AEROTP_DataReceive_VEH_DYN(const char* topic_name_, const class aero_vehicle_data_protobuf::AERO_t_VehicleDynamics_Proto& msg_, const long long time_)
{
  AEROVehicleDataProto_DeSerializeVehicleDynamics(&msg_,  &VEH_DYN_Data); topic_name_; time_;
}


void CModPlugIn::AEROTP_DataReceive_GPS(const char* topic_name_, const class aero_gps_rmc_protobuf::AERO_t_GpsRmc_Proto& msg_, const long long time_)
{
  AEROGpsRmcProto_DeSerialize(&msg_,  &GPS_Data); topic_name_; time_;
}


//////////////////////////////////////////////////////////
//           class instance
//////////////////////////////////////////////////////////
//! Creates a CModPlugIn class instance (do not change)
CBasePlugIn* GetPlugIn(const char* inst_name, wxPlg_IntFunT callback)
{
   return(new CModPlugIn(inst_name, callback));
}