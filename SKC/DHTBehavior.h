#ifdef __MRAA__
#ifndef DHTBEHAVIOR_H_INCLUDED
#define DHTBEHAVIOR_H_INCLUDED

#include "SKCLib/SensorBehavior.h"
#include "Sensors/DHT.h"
#include "SKCLib/PowerPin.h"

class DHTBehavior : public SensorBehavior, PowerPin{
private:
    DHT * dht = NULL;

public:
    DHTBehavior(uint8_t inpin,uint8_t outpin, uint8_t powerPin, string id, int delay, int repeat) : SensorBehavior("DHT",id), PowerPin(powerPin){
        cout << "[" << GetName() << "::" << GetID() <<  "] Constructing..." << endl;

        dht = new DHT(inpin,outpin,11);

        SetTimeCritical(true);
        SetSensorUpdateDelay(repeat);
        SetSensorSetupDelay(delay);
        SetEnabled(true);
    }

    ~DHTBehavior(){
        if(dht!=NULL) delete dht;
    }


	vector<IRestRequest*> Update(){

        cout << "[" << GetName() << "::" << GetID() <<  "] Reading..." << endl;

        vector<IRestRequest*> ret = vector<IRestRequest*>();
        int tries = 0;
        skc::CyberPhysicalDescriptor cpd;

        OrionUpdateContextRequest * urs = new OrionUpdateContextRequest("http://posttestserver.com/",cpd);

        while(!dht->read() && tries < 3) tries++;
        if(tries >= 3) return ret;


        urs->AddData("Humidity",dht->readHumidity());
        urs->AddData("Temperature",dht->readTemperature());
        ret.push_back((IRestRequest*)urs);

        cout << "[" << GetName() << "::" << GetID() <<  "] Updating..." << endl;
        return ret;
	};


    void PowerUp(){
        PowerPin::PowerUp();
    }
    void PowerDown(){
        PowerPin::PowerDown();
    }


};


#endif // DHTBEHAVIOR_H_INCLUDED
#endif
