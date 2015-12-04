#include <pthread.h>
#include <boost/bind.hpp>
//#include <mraa.h>

#include <boost/type_traits/is_base_of.hpp>
#include <boost/static_assert.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include "InlineHelper.h"
#include "SensorManager.h"
#include "Vectors.h"


namespace skc{


void SensorManager::Finished(){
    _io_service.stop();
	_threadpool.join_all();
}

//checks sensors, is a thread call
void SensorManager::Update(){
    long ms_to_next=0;
    //cout << "Sensor count: " << _sensors.size() << endl;

    for (std::vector<Sensor>::iterator it = _sensors.begin(); it != _sensors.end(); ++it){
        ms_to_next = (*it)._gs->GetSensorUpdateDelay() - InlineHelper::GetMillisecondsSince(&(*it)._lastRun);
        boost::posix_time::ptime c_next = InlineHelper::GetTime(ms_to_next);
        if(c_next < _t_next_sensor) _t_next_sensor = c_next;

        if ((*it)._gs->GetEnabled() && !(*it)._gs->_running && ms_to_next <= 0){
            (*it)._gs->_running = true;
            startASensor(&(*it));
        }


//        if ((*it)._gs->GetEnabled() && !(*it)._gs->_running && InlineHelper::GetMillisecondsSince(&(*it)._lastRun) >= (*it)._gs->GetSensorUpdateDelay()){
 //           (*it)._gs->_running = true;
 //           startASensor(&(*it));
 //       }
    }
    ms_to_next = -InlineHelper::GetMillisecondsSince(&_t_next_sensor);
    //if(ms_to_next > 600000) ms_to_next = 600000; //max of 10minutes beofre checking again
    if(ms_to_next > 5000 && GetRunningSensorCount()==0 && _queue->empty()){
        cout << "[SensorManager] Stopping all threads, no sensor and net activity for the next " << ms_to_next << " milliseconds." << endl;
        InterruptableThread::PauseAllThreads(ms_to_next - 4000); //vai se pausar!!!! cuidado!
    }

    _t_next_sensor = boost::posix_time::pos_infin;


}





inline void SensorManager::startASensor(Sensor * s){
	///TODO se for timecritical, tem que parar todo o resto
	if(s->_gs->_instantStart)
		_threadpool.add_thread(new boost::thread(boost::bind(&SensorManager::SensorThread, this, s))); //para execucao imediata
	else
		_io_service.post(boost::bind(&SensorManager::SensorThread, this, s)); //para pool
}

void SensorManager::SensorThread(Sensor * s){

    _active_sensors.fetch_add(1,boost::memory_order_relaxed);

    cout << "[" << s->_gs->GetName() << "] Sensor thread" << endl;
    if(s->_gs->_timeCritical){
        //mraa_set_priority(99); //set thread to maximum level (real time)
        //status = pthread_getschedparam(pthread_self(), &policy, &param);
        //if(status!=0) status=-1;
        int policy, status;
        struct sched_param param;
        policy = SCHED_FIFO;
        param.__sched_priority = 99;
        status = pthread_setschedparam(pthread_self(), policy, &param);
        if(status!=0) printf("[Error] Could not rise thread priority for time sensitive sensor."); ///TODO ERROR HANDLING
    }


	vector<IRestRequest*> rs = vector<IRestRequest*>();

	//so pode comecar como undefined (desligado)
	//ou ready, pros alwayson
	if (s->_status != SensorStatus::Ready && s->_status != SensorStatus::undefined){
		if (s->_unresponsiveTime.is_pos_infinity()) s->_unresponsiveTime = InlineHelper::GetTime();
		return;
	}

	s->_unresponsiveTime = boost::posix_time::pos_infin;
	//s->_lastStart = InlineHelper::GetTime(); UPDATE feiot anteriormente (no _Run)


	if (s->_status != SensorStatus::Ready){
		boost::posix_time::ptime t_start;
		s->_status = SensorStatus::Busy;
		////TODO: ligar device
		s->_gs->PowerUp();
		s->_status = SensorStatus::Starting;

		rs += s->_gs->Start();
        t_start = InlineHelper::GetTime();
		s->_status = SensorStatus::Busy;
		//while (1);
		///TODO: esperar o tempo ate update
		long wait_ms = s->_gs->GetSensorSetupDelay() - InlineHelper::GetMillisecondsSince(&(t_start));
		cout << "Waiting for " << wait_ms << "ms before Update" << endl;
		if(wait_ms > 0);
        boost::this_thread::sleep(boost::posix_time::milliseconds(wait_ms));
	}


	s->_status = SensorStatus::Updating;
	rs += s->_gs->Update();

	if (!s->_gs->GetAlwaysOn()){
		s->_status = SensorStatus::Stoping;
		rs += s->_gs->Stop();
		////TODO: desligar device
		s->_gs->PowerDown();
		s->_status = SensorStatus::undefined;
	}

	//add requests to client queue
	for (std::vector<IRestRequest*>::iterator it = rs.begin(); it != rs.end(); ++it) _queue->add((*it));

    s->_lastRun = InlineHelper::GetTime();
    s->_gs->_running = false;

    _active_sensors.fetch_sub(1,boost::memory_order_relaxed);
}

SensorManager::SensorManager(RESTQueue * queue){
	_sensors.reserve(8); //reserve space for 8 sensors, will hardly ever reach 8
	_queue = queue;
	_work = new boost::asio::io_service::work(_io_service);

    _t_next_sensor = boost::posix_time::pos_infin;

	//setup threads for threadpool
	for(int i=0; i<4; i++) _threadpool.create_thread(boost::bind(static_cast<size_t (boost::asio::io_service::*)()>(&boost::asio::io_service::run), &_io_service));
}

SensorManager::~SensorManager(){

	_io_service.stop();
	_work->~work();
	delete _work;
	_threadpool.join_all();

    //delete all sensors
    for (std::vector<Sensor>::iterator it = _sensors.begin(); it != _sensors.end(); ++it) delete (*it)._gs;
	_sensors.clear();
}

void SensorManager::RemoveSensor(string id){
	std::vector<Sensor>::iterator it;
	std::vector<Sensor>::iterator * r = NULL;
	for (it = _sensors.begin(); it != _sensors.end(); ++it){
		if ((*it)._gs->GetID() == id)  r = &it;
	}
	if(r!=NULL) _sensors.erase(*r);
}


int SensorManager::GetRunningSensorCount(){
    return _active_sensors.fetch_add(0,boost::memory_order_relaxed);
}

};




/*

namespace skc{

bool SensorManager::Start(){
	if (thManager != NULL) return false;
	_enabled = true;
	_run = true;
	thManager = new boost::thread(boost::bind(&SensorManager::_Run, this)); //inicia thread da propria classe
	return true;
}

void SensorManager::Stop(){
	_enabled = false;
	_run = false;
	thManager->join();
	delete thManager;
	thManager = NULL;
	_io_service.stop();
	_threadpool.join_all();
}

void SensorManager::SetEnabled(bool enable){
	_enabled = enable;
}
bool SensorManager::GetEnabled(){
	return _enabled;
}

//checks sensors, is a thread
void SensorManager::_Run(){

	while (_run){
		while (!_enabled);
		while (_enabled){
			for (std::vector<Sensor>::iterator it = _sensors.begin(); it != _sensors.end(); ++it){
				if ((*it)._gs->GetEnabled() && !(*it)._gs->_running && InlineHelper::GetMillisecondsSince(&(*it)._lastRun) >= (*it)._gs->GetSensorUpdateDelay()){
					(*it)._gs->_running = true;
					startASensor(&(*it));
				}
			}
		}
	}
}





inline void SensorManager::startASensor(Sensor * s){
	///TODO se for timecritical, tem que parar todo o resto
	if(s->_gs->_instantStart)
		_threadpool.add_thread(new boost::thread(boost::bind(&SensorManager::SensorThread, this, s))); //para execucao imediata
	else
		_io_service.post(boost::bind(&SensorManager::SensorThread, this, s)); //para pool
}

void SensorManager::SensorThread(Sensor * s){
    //s->_gs->
    cout << "[" << s->_gs->GetName() << "] Sensor thread" << endl;
    if(s->_gs->_timeCritical){
        //mraa_set_priority(99); //set thread to maximum level (real time)
        //status = pthread_getschedparam(pthread_self(), &policy, &param);
        //if(status!=0) status=-1;
        int policy, status;
        struct sched_param param;

        policy = SCHED_FIFO;
        param.__sched_priority = 99;
        status = pthread_setschedparam(pthread_self(), policy, &param);
        if(status!=0) printf("[Error] Could not rise thread priority for time sensitive sensor."); ///TODO ERROR HANDLING
    }


	vector<IRestRequest*> rs = vector<IRestRequest*>();

	//so pode comecar como undefined (desligado)
	//ou ready, pros alwayson
	if (s->_status != SensorStatus::Ready && s->_status != SensorStatus::undefined){
		if (s->_unresponsiveTime.is_pos_infinity()) s->_unresponsiveTime = InlineHelper::GetTime();
		return;
	}

	s->_unresponsiveTime = boost::posix_time::pos_infin;
	//s->_lastStart = InlineHelper::GetTime(); UPDATE feiot anteriormente (no _Run)


	if (s->_status != SensorStatus::Ready){
		boost::posix_time::ptime t_start;
		s->_status = SensorStatus::Busy;
		///TODO: ligar device
		s->_status = SensorStatus::Starting;

		rs += s->_gs->Start();
        t_start = InlineHelper::GetTime();
		s->_status = SensorStatus::Busy;
		//while (1);
		///TODO: esperar o tempo ate update
		long wait_ms = s->_gs->GetSensorSetupDelay() - InlineHelper::GetMillisecondsSince(&(t_start));
		cout << "Waiting for " << wait_ms << "ms before Update" << endl;
		if(wait_ms > 0);
        boost::this_thread::sleep(boost::posix_time::milliseconds(wait_ms));
	}


	s->_status = SensorStatus::Updating;
	rs += s->_gs->Update();

	if (!s->_gs->GetAlwaysOn()){
		s->_status = SensorStatus::Stoping;
		rs += s->_gs->Stop();
		///TODO: desligar device
		s->_status = SensorStatus::undefined;
	}

    cout << "Itens em rs: " << rs.size() << endl;
	//add requests to client queue
	for (std::vector<IRestRequest*>::iterator it = rs.begin(); it != rs.end(); ++it) _queue->add((*it));

    s->_lastRun = InlineHelper::GetTime();
    s->_gs->_running = false;
}

SensorManager::SensorManager(RESTQueue * queue){
	_sensors.reserve(8); //reserve space for 8 sensors, will hardly ever reach 8
	_queue = queue;
	_work = new boost::asio::io_service::work(_io_service);

	//setup threads for threadpool
	for(int i=0; i<4; i++) _threadpool.create_thread(boost::bind(static_cast<size_t (boost::asio::io_service::*)()>(&boost::asio::io_service::run), &_io_service));
}

SensorManager::~SensorManager(){

	_io_service.stop();
	_work->~work();
	delete _work;
	_threadpool.join_all();

    //delete all sensors
    for (std::vector<Sensor>::iterator it = _sensors.begin(); it != _sensors.end(); ++it) delete (*it)._gs;
	_sensors.clear();
}

void SensorManager::RemoveSensor(string id){
	std::vector<Sensor>::iterator it;
	std::vector<Sensor>::iterator * r = NULL;
	for (it = _sensors.begin(); it != _sensors.end(); ++it){
		if ((*it)._gs->GetID() == id)  r = &it;
	}
	if(r!=NULL) _sensors.erase(*r);
}


};


*/
