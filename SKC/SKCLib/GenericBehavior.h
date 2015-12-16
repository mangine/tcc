#ifndef GENERICBEHAVIOR_H_INCLUDED
#define GENERICBEHAVIOR_H_INCLUDED

#include <random>
#include <cmath>

#include "SensorBehavior.h"
#include "IRestRequest.h"
#include "OrionRestRequest.h"
#include "OrionUpdateContextRequest.h"
#include "CyberPhysicalDescriptor.h"

#include "../globals.h"

using namespace skc;
using namespace skc::RESTRequest;

class GenericBehavior : public skc::SensorBehavior{
private:
    string _namev;
    float _value;
    float _var;

public:

    GenericBehavior(string name, string id, string nomev, float value, float var, int delay, int repeat) : SensorBehavior(name,id){
        _namev = nomev;
        _value = value;
        _var = var;
        cout << "[" << GetName() << "::" << GetID() <<  "] Constructing..." << endl;
        SetSensorUpdateDelay(repeat);
        SetSensorSetupDelay(delay);
        SetEnabled(true);
    }

    vector<IRestRequest*> Start(){
        vector<IRestRequest*> ret = vector<IRestRequest*>();
        cout << "[" << GetName() << "::" << GetID() << "] Starting..." << endl;

        float v = ((int)(_value + _var*rand())*100)/100;

        skc::CyberPhysicalDescriptor cpd;
        cpd.id = "Pote1";
        cpd.isPattern= false;
        cpd.name = "Pote1";
        cpd.type = "Pote";

        OrionUpdateContextRequest * urs = new OrionUpdateContextRequest(_orionURL,cpd);
        urs->AddData(_namev.c_str(),v);
        urs->SetCallback(boost::bind(&GenericBehavior::teste_callback,this,_1));
        ret.push_back((IRestRequest*)urs);

        /*OrionRestRequest * ors = new OrionRestRequest("");
        ors->SetMethod(web::http::methods::GET);
        ors->AddData("valorfloat",10.5f);
        ors->SetCallback(boost::bind(&test_behavior::teste_callback,this,_1));
        ret.push_back((IRestRequest*)ors);
        */
//        ors->SetCallback([](web::http::http_response r){cout << "Resposta de request: " << r.status_code() << endl;});
 //       ors->SetCallback(boost::bind(&test_behavior::teste_callback,this,_1));

//        ors->SetCallback(boost::function<void(web::http::http_response)>(teste_callback));

        //return vector<IRestRequest*>();
        return ret;

    } //call right after turning device on

	vector<IRestRequest*> Stop(){
        cout << "[" << GetName() << "::" << GetID() <<  "] Stopping..." << endl;
        return vector<IRestRequest*>();
    }; //call right before turning device off

	vector<IRestRequest*> Update(){
        cout << "[" << GetName() << "::" << GetID() <<  "] Updating..." << endl;
        return vector<IRestRequest*>();
	}; //call to gather data

	vector<IRestRequest*> Reset(){
        cout << "[" << GetName() << "::" << GetID() <<  "] Reseting..." << endl;
        return vector<IRestRequest*>();
    }; //reset device (THIS DOESNT POWER OFF THE DEVICE)

    void teste_callback(web::http::http_response r){
        cout << "Resposta de request: " << r.status_code() << endl;
        cout << "-Body: " << r.to_string();
    }

};




#endif // GENERICBEHAVIOR_H_INCLUDED
