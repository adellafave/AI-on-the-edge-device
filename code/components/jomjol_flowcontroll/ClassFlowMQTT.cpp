#include <sstream>
#include "ClassFlowMQTT.h"
#include "Helper.h"
#include "connect_wlan.h"

#include "time_sntp.h"
#include "interface_mqtt.h"
#include "ClassFlowPostProcessing.h"
#include "ClassLogFile.h"

#include <iomanip>
#include <sstream>

#include <time.h>

void ClassFlowMQTT::SetInitialParameter(void)
{
    uri = "";
    topic = "";
    topicError = "";
    topicRate = "";
    topicTimeStamp = "";
    maintopic = "";
    mainerrortopic = ""; 

    topicUptime = "";
    topicFreeMem = "";
    clientname = "watermeter";
    OldValue = "";
    flowpostprocessing = NULL;  
    user = "";
    password = ""; 
    SetRetainFlag = 0;  
    previousElement = NULL;
    ListFlowControll = NULL; 
    disabled = false;
    MQTTenable = false;
    keepAlive = 600; // TODO This must be greater than the Flow Interval!
}       

ClassFlowMQTT::ClassFlowMQTT()
{
    SetInitialParameter();
}

ClassFlowMQTT::ClassFlowMQTT(std::vector<ClassFlow*>* lfc)
{
    SetInitialParameter();

    ListFlowControll = lfc;
    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }
    }
}

ClassFlowMQTT::ClassFlowMQTT(std::vector<ClassFlow*>* lfc, ClassFlow *_prev)
{
    SetInitialParameter();

    previousElement = _prev;
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("ClassFlowPostProcessing") == 0)
        {
            flowpostprocessing = (ClassFlowPostProcessing*) (*ListFlowControll)[i];
        }
    }
}


bool ClassFlowMQTT::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (toUpper(aktparamgraph).compare("[MQTT]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((toUpper(zerlegt[0]) == "USER") && (zerlegt.size() > 1))
        {
            this->user = zerlegt[1];
        }  
        if ((toUpper(zerlegt[0]) == "PASSWORD") && (zerlegt.size() > 1))
        {
            this->password = zerlegt[1];
        }               
        if ((toUpper(zerlegt[0]) == "URI") && (zerlegt.size() > 1))
        {
            this->uri = zerlegt[1];
        }
        if ((toUpper(zerlegt[0]) == "SETRETAINFLAG") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SetRetainFlag = 1;  
        }


        if ((toUpper(zerlegt[0]) == "CLIENTID") && (zerlegt.size() > 1))
        {
            this->clientname = zerlegt[1];
        }

        if (((toUpper(zerlegt[0]) == "TOPIC") || (toUpper(zerlegt[0]) == "MAINTOPIC")) && (zerlegt.size() > 1))
        {
            maintopic = zerlegt[1];
        }
    }

    printf("Init Read with uri: %s, clientname: %s, user: %s, password: %s, maintopic: %s\n", uri.c_str(), clientname.c_str(), user.c_str(), password.c_str(), mainerrortopic.c_str());
    if (!MQTTisConnected() && (uri.length() > 0) && (maintopic.length() > 0)) 
    { 
        printf("InitMQTTInit\n");
        mainerrortopic = maintopic + "/connection";
        printf("Init MQTT with uri: %s, clientname: %s, user: %s, password: %s, maintopic: %s\n", uri.c_str(), clientname.c_str(), user.c_str(), password.c_str(), mainerrortopic.c_str());
        if (!MQTTInit(uri, clientname, user, password, mainerrortopic, keepAlive))
        { // Failed
            MQTTenable = false;
            return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
        }
    }

    // Try sending mainerrortopic. If it fails, re-run init
    if (!MQTTPublish(mainerrortopic, "connected", SetRetainFlag))
    { // Failed
        LogFile.WriteToFile("MQTT - Re-running init...!");
        if (!MQTTInit(this->uri, this->clientname, this->user, this->password, this->mainerrortopic, keepAlive))
        { // Failed
            MQTTenable = false;
            return false;
        } 
    }

    // Try again and quit if it fails
    if (!MQTTPublish(mainerrortopic, "connected", SetRetainFlag))
    { // Failed
        MQTTenable = false;
        return false;
    }



   
 /*   if (!MQTTPublish(mainerrortopic, "connected", SetRetainFlag))
    { // Failed
        LogFile.WriteToFile("MQTT - Could not publish connection status!");
        MQTTenable = false;
        return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
    }*/

 /*   if(!MQTTPublish(_LWTContext, "", 1))
    {
        LogFile.WriteToFile("MQTT - Could not publish LWT!");
        MQTTenable = false;
        return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
    }*/


    MQTTenable = true;
    return true;
}


string ClassFlowMQTT::GetMQTTMainTopic()
{
    return maintopic;
}


bool ClassFlowMQTT::doFlow(string zwtime)
{
  //  if (!MQTTenable) {
  //      LogFile.WriteToFile("MQTT not enabled!");
  //
  //      // Try again to init it
  //   if (!MQTTInit(this->uri, this->clientname, this->user, this->password, this->mainerrortopic, keepAlive))
  //      { // Failed
  //          MQTTenable = false;
  //          return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
  //      } 
  //
  //     if (!MQTTPublish(mainerrortopic, "connected", SetRetainFlag))
  //      { // Failed
  //          MQTTenable = false;
  //          return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
  //      }
  //      
  //      LogFile.WriteToFile("MQTT is now enabled");
  //      MQTTenable = true;
  //  }


    // Try sending mainerrortopic. If it fails, re-run init
    if (!MQTTPublish(mainerrortopic, "connected", SetRetainFlag))
    { // Failed
        LogFile.WriteToFile("MQTT - Re-running init...!");
        if (!MQTTInit(this->uri, this->clientname, this->user, this->password, this->mainerrortopic, keepAlive))
        { // Failed
            MQTTenable = false;
            return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
        } 
    }

    // Try again and quit if it fails
    if (!MQTTPublish(mainerrortopic, "connected", SetRetainFlag))
    { // Failed
        MQTTenable = false;
        return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
    }

    std::string result;
    std::string resulterror = "";
    std::string resultraw = "";
    std::string resultprevalue = "";
    std::string resultrate = "";
    std::string resulttimestamp = "";
    std::string resultchangabs = "";
    std::string resultROI = "";
    string zw = "";
    string namenumber = "";

    // if (!MQTTPublish(mainerrortopic, "connected", SetRetainFlag))
    //{ // Failed, skip other topics
    //    return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
    //}
    
    zw = maintopic + "/" + "uptime";
    char uptimeStr[11];
    sprintf(uptimeStr, "%ld", (long)getUpTime());
    MQTTPublish(zw, uptimeStr, SetRetainFlag);

    zw = maintopic + "/" + "freeMem";
    char freeheapmem[11];
    sprintf(freeheapmem, "%zu", esp_get_free_heap_size());
    if (!MQTTPublish(zw, freeheapmem, SetRetainFlag))
    { // Failed, skip other topics
        return true; // We need to return true despite we failed, else it will retry 5x and then reboot!
    }

    zw = maintopic + "/" + "wifiRSSI";
    char rssi[11];
    sprintf(rssi, "%d", get_WIFI_RSSI());
    MQTTPublish(zw, rssi, SetRetainFlag);

    zw = maintopic + "/" + "CPUtemp";
    std::string cputemp = std::to_string(temperatureRead());
    MQTTPublish(zw, cputemp, SetRetainFlag);

    if (flowpostprocessing)
    {
        std::vector<NumberPost*>* NUMBERS = flowpostprocessing->GetNumbers();

        for (int i = 0; i < (*NUMBERS).size(); ++i)
        {
            result =  (*NUMBERS)[i]->ReturnValue;
            resultraw =  (*NUMBERS)[i]->ReturnRawValue;
            resultprevalue =  (*NUMBERS)[i]->ReturnPreValue;
            resulterror = (*NUMBERS)[i]->ErrorMessageText;
            resultrate = (*NUMBERS)[i]->ReturnRateValue;
            resultchangabs = (*NUMBERS)[i]->ReturnChangeAbsolute;
            resulttimestamp = (*NUMBERS)[i]->timeStamp;

            namenumber = (*NUMBERS)[i]->name;
            if (namenumber == "default")
                namenumber = maintopic + "/";
            else
                namenumber = maintopic + "/" + namenumber + "/";

            zw = namenumber + "value"; 
            if (result.length() > 0)   
                MQTTPublish(zw, result, SetRetainFlag);

            zw = namenumber + "error"; 
            if (resulterror.length() > 0)  
                MQTTPublish(zw, resulterror, SetRetainFlag);

            zw = namenumber + "rate"; 
            if (resultrate.length() > 0)   
                MQTTPublish(zw, resultrate, SetRetainFlag);

            zw = namenumber + "changeabsolut"; 
            if (resultchangabs.length() > 0)   
                MQTTPublish(zw, resultchangabs, SetRetainFlag);

            zw = namenumber + "raw"; 
            if (resultraw.length() > 0)   
                MQTTPublish(zw, resultraw, SetRetainFlag);

            zw = namenumber + "prevalue"; 
            if (resultprevalue.length() > 0)   
                MQTTPublish(zw, resultprevalue, SetRetainFlag);

            zw = namenumber + "timestamp";
            if (resulttimestamp.length() > 0)
                MQTTPublish(zw, resulttimestamp, SetRetainFlag);

            std::string json = "";
            
            if (result.length() > 0)
                json += "{\"value\":"+result;
            else
                json += "{\"value\":\"\"";

            json += ",\"raw\":"+resultraw;
            json += ",\"prevalue\":"+resultprevalue;
            json += ",\"error\":\""+resulterror+"\"";


            if ((*NUMBERS)[i]->analog_roi)
            {
                for (int j = 0; j < (*NUMBERS)[i]->analog_roi->ROI.size(); ++j)
                {
                    resultROI = RundeOutput((*NUMBERS)[i]->analog_roi->ROI[j]->result_float,1);
                    zw = namenumber + (*NUMBERS)[i]->analog_roi->ROI[j]->name; 
                    if (resultROI.length() > 0)  
                    {
                        json += ",\"" + (*NUMBERS)[i]->analog_roi->ROI[j]->name + "\":" + resultROI;
                        MQTTPublish(zw, resultROI, SetRetainFlag);
                    }
                }
            }

            if ((*NUMBERS)[i]->digit_roi)
            {
                for (int j = 0; j < (*NUMBERS)[i]->digit_roi->ROI.size(); ++j)
                {
                    resultROI = RundeOutput((*NUMBERS)[i]->digit_roi->ROI[j]->result_float,1);
                    zw = namenumber + (*NUMBERS)[i]->digit_roi->ROI[j]->name; 
                    if (resultROI.length() > 0)  
                    {
                        json += ",\"" + (*NUMBERS)[i]->digit_roi->ROI[j]->name + "\":" + resultROI;
                        MQTTPublish(zw, resultROI, SetRetainFlag);
                    }
                }
            }

            if (resultrate.length() > 0)
                json += ",\"rate\":"+resultrate;
            else
                json += ",\"rate\":\"\"";
            json += ",\"timestamp\":\""+resulttimestamp+"\"}";

            zw = namenumber + "json";
            MQTTPublish(zw, json, SetRetainFlag);
        }
    }
    else
    {
        for (int i = 0; i < ListFlowControll->size(); ++i)
        {
            zw = (*ListFlowControll)[i]->getReadout();
            if (zw.length() > 0)
            {
                if (result.length() == 0)
                    result = zw;
                else
                    result = result + "\t" + zw;
            }
        }
        MQTTPublish(topic, result, SetRetainFlag);
    }
    
    OldValue = result;
    
    return true;
}

string ClassFlowMQTT::RundeOutput(float _in, int _anzNachkomma){
    std::stringstream stream;
    int _zw = _in;    
//    printf("AnzNachkomma: %d\n", _anzNachkomma);

    if (_anzNachkomma < 0) {
        _anzNachkomma = 0;
    }

    if (_anzNachkomma > 0)
    {
        stream << std::fixed << std::setprecision(_anzNachkomma) << _in;
        return stream.str();          
    }
    else
    {
        stream << _zw;
    }


    return stream.str();  
}