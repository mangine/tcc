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


    #ifdef __MRAA__
    MQ135Behavior * m = new (sm.AddSensor<MQ135Behavior>())MQ135Behavior(1,8,"id1",60000,300000);
    DHTBehavior * d = new (sm.AddSensor<DHTBehavior>())DHTBehavior(2,3,9,"id1",1000,60000);
    #else
    test_behavior * t = new (sm.AddSensor<test_behavior>())test_behavior("60s-start-5m-update",60000,300000);
    test_behavior * t2 = new (sm.AddSensor<test_behavior>())test_behavior("1s-start-1m-update",1000,60000);
    #endif // __MRAA__

    sm.Launch();
    rc.Launch();
  /* Code in this loop will run repeatedly
   */

  for (;;) {

  }
    return 0;
}
