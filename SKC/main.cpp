#include <iostream>

using namespace std;

#include <cpprest/json.h>
#include <boost/filesystem.hpp>

#include "SKCLib/OrionRestRequest.h"
#include "SKCLib/RESTQueue.h"
#include "SKCLib/RESTClient.h"
#include "SKCLib/SensorManager.h"
#include "SKCLib/TestBehavior.h"
#include "SKCLib/CyberPhysicalDescriptor.h"

#include "MQ135Behavior.h"
#include "DHTBehavior.h"

#include "globals.h"

skc::CyberPhysicalDescriptor* skc::CyberPhysicalDescriptor::_default_cpd = NULL;

boost::filesystem::path _exe_path;



inline bool file_exists_fast(const string& name){
    struct stat buffer;
    return (stat(name.c_str(),&buffer) == 0);
}

void quit_program(int status = 0){
    cout << endl << "Press any key to exit" << endl;
    getchar();
    exit(status);
}

void create_configuration_file(){
    ofstream f("config.json");

    if(!f.is_open()){
        cout << "[ERROR] Couldn't create configuration file. Check for permissions and try again." << endl;
    }

    web::json::value file_json;
    file_json["context"] = web::json::value();
    file_json["context"]["type"] = web::json::value::string(U(""));
    file_json["context"]["name"] = web::json::value::string(U(""));
    file_json["context"]["isPatern"] = web::json::value::boolean(false);
    file_json["context"]["parent"] = web::json::value::string(U(""));

    file_json["orion"] = web::json::value();
    file_json["orion"]["url"] = web::json::value::string(U(""));

    f << file_json.serialize();
    f.close();

}

void load_configuration_file(){
    ifstream f("config.json");
    web::json::value file_json;

    if(!boost::filesystem::exists("config.json")){
        create_configuration_file();
        cout << "[ERROR] Couldn't find config.json. File was created at " <<  _exe_path.stem().c_str() << endl << "Please fill the file with your configurations and restart the program." << endl;
    }

    if(!f.is_open()){
        cout << "[ERROR] Couldn't create configuration file. Check for permissions and try again." << endl;
        quit_program(1);
    }

    std::string str((std::istreambuf_iterator<char>(f)),
                 std::istreambuf_iterator<char>());

    f.close();
    file_json = web::json::value::parse(str);

    try{
        skc::CyberPhysicalDescriptor::_default_cpd = new CyberPhysicalDescriptor();
        skc::CyberPhysicalDescriptor::_default_cpd->type = file_json["context"]["type"].as_string();
        skc::CyberPhysicalDescriptor::_default_cpd->name = file_json["context"]["name"].as_string();
        skc::CyberPhysicalDescriptor::_default_cpd->isPattern = file_json["context"]["isPatern"].as_bool();
        skc::CyberPhysicalDescriptor::_default_cpd->parentName = file_json["context"]["parent"].as_string();
        _orionURL = file_json["orion"]["url"].as_string();

    }catch(...){
        cout << "[ERROR] Error reading json file, it will be recreated. Restart the program." << endl;
        if(remove("config.json")!=0){
                cout << "[ERROR] Error deleting config.json file. Check for permission and try again, or delete it manualy." << endl;
                quit_program(3);
        }else{
            create_configuration_file();
            quit_program(0);
        }
    }
}



int main(int argc, char** argv)
{

    //_exe_path = boost::filesystem::path(boost::filesystem::current_path<boost::filesystem::path>());

    _exe_path = boost::filesystem::current_path();
    load_configuration_file();
    //cout << "Test finished, sleeping for 20s before launching program..." << endl << endl<< endl;
    //sleep(20);


    skc::RESTQueue rq(16);
    skc::SensorManager sm(&rq);
    skc::RESTClient rc(&rq);


    #ifdef __MRAA__
    MQ135Behavior * m = new (sm.AddSensor<MQ135Behavior>())MQ135Behavior(6,7,"id1",60000,300000);
    DHTBehavior * d = new (sm.AddSensor<DHTBehavior>())DHTBehavior(8,9,10,"id1",2000,60000);
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
