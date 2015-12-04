#include <iostream>

using namespace std;

#include "SKCLib/OrionRestRequest.h"
#include "SKCLib/RESTQueue.h"
#include "SKCLib/RESTClient.h"
#include "SKCLib/SensorManager.h"
#include "SKCLib/TestBehavior.h"


#include "MQ135Behavior.h"
#include "DHTBehavior.h"


int main()
{
    skc::RESTQueue rq(16);
    skc::SensorManager sm(&rq);
    skc::RESTClient rc(&rq);

    //test_behavior * t = new (sm.AddSensor<test_behavior>())test_behavior("id1",1000,20000);
    #ifdef __MRAA__
    MQ135Behavior * m = new (sm.AddSensor<MQ135Behavior>())MQ135Behavior(1,8,"id1",60000,300000);
    DHTBehavior * d = new (sm.AddSensor<DHTBehavior>())DHTBehavior(2,3,9,"id1",1000,60000);
    #endif // __MRAA__

    sm.Launch();
    rc.Launch();
  /* Code in this loop will run repeatedly
   */

  for (;;) {

  }
    return 0;
}
