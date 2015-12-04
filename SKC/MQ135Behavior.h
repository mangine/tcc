#ifdef __MRAA__
#ifndef MQ135BEHAVIOR_H_INCLUDED
#define MQ135BEHAVIOR_H_INCLUDED

#include "SKCLib/SensorBehavior.h"
#include "Sensors/mq135.h"
#include "SKCLib/PowerPin.h"

class MQ135Behavior : public SensorBehavior, PowerPin{
private:
    MQ135 * mq = NULL;

public:
    MQ135Behavior(uint8_t pin, uint8_t powerPin, string id, int delay, int repeat) : SensorBehavior("MQ135",id), PowerPin(powerPin){
        cout << "[" << GetName() << "::" << GetID() <<  "] Constructing..." << endl;

        mq = new MQ135(pin);
        SetSensorUpdateDelay(repeat);
        SetSensorSetupDelay(delay);
        SetEnabled(true);
    }

    ~MQ135Behavior(){
        if(mq!=NULL) delete mq;
    }


	vector<IRestRequest*> Update(){

        cout << "[" << GetName() << "::" << GetID() <<  "] Reading..." << endl;


        vector<IRestRequest*> ret();
        skc::CyberPhysicalDescriptor cpd;
        OrionUpdateContextRequest * urs = new OrionUpdateContextRequest("http://posttestserver.com/post.php?dir=/",cpd);
        urs->AddData("AmoniaPPM",mq->getCorrectedPPM());
//        urs->SetCallback(boost::bind(&test_behavior::teste_callback,this,_1));
        ret.push_back((IRestRequest*)urs);

        cout << "[" << GetName() << "::" << GetID() <<  "] Updating..." << endl;
        return ret;
	}; //call to gather data


    void PowerUp(){
        PowerPin::PowerUp();
    }
    void PowerDown(){
        PowerPin::PowerDown();
    }


};

#endif // MQ135BEHAVIOR_H_INCLUDED
#endif
