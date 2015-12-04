#ifndef INTERRUPTABLETHREAD_INCLUDED
#define INTERRUPTABLETHREAD_INCLUDED

#include <signal.h>

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include <vector>

///TODO cuidar do missed awake

using namespace std;

class InterruptableThread{

public:
    enum ThreadState{Uninitialized, Running, PausingTimeout, Pausing, Paused, PausedTimeout, Stopping, Stopped};

    static void StopAllThreads();
    static void PauseAllThreads();
    static void PauseAllThreads(long ms);
    static int GetRunningThreads();
    static void ResumeAllThreads();
    static vector<InterruptableThread*> GetAllThreads();
    static InterruptableThread * GetThreadByID(boost::thread::id id);

    InterruptableThread();
    ~InterruptableThread();
    bool Launch(); //launches thread
    bool Pause();
    bool Pause(long milliseconds);
    void Resume(); //resumes thread
    bool Stop(bool force=false); //finishes thread

    bool GetInterruptionEnabled();
    void SetInterruptionEnabled(bool enabled);

    ThreadState GetThreadState(){
        return _state;
    }
    bool IsInterrupted(){
        return _interrupted;
    }
    boost::thread::id GetThreadID(){
        return _thisThread->get_id();
    }

    int GetPriority();
    void SetPriority(int value);
private:
    struct InterruptRequest{
        ThreadState _target_state = ThreadState::Uninitialized;
        long _pause_timeout = 0;
        bool _active = false;
    };

    static vector<InterruptableThread*> _threadObjs;
    static boost::atomic<int> _running_threads;

    boost::thread * _thisThread = NULL;
    boost::mutex _mutex;
    boost::condition_variable _cv;
    boost::atomic<bool> _interrupted;

    bool _interruption_enabled = true;
    InterruptRequest _interrupt_request;
    ThreadState _state;

    void _run();

protected:
//static boost::atomic<int> _running_threads;
    boost::mutex * const GetMutex();

public:
    //sobreescreviveis
    virtual bool CanLaunch(){return true;} //called before launchingm to user validation
    virtual void Update(){}; //thread thread
    virtual void Start(){}; //thread start call
    virtual void Finished(){};
    virtual void OnPause(){}; //just before pausing
    virtual void OnResume(){}; //just after resuming
};
/*
vector<InterruptableThread*> InterruptableThread::_threadObjs = vector<InterruptableThread *>();
boost::atomic<int> InterruptableThread::_running_threads(0);


boost::mutex * const InterruptableThread::GetMutex(){
    return &_mutex;
}

void InterruptableThread::StopAllThreads(){
    for(vector<InterruptableThread*>::const_iterator it = _threadObjs.begin();it!=_threadObjs.end();++it) (*it)->Stop();
}
void InterruptableThread::PauseAllThreads(){
    for(vector<InterruptableThread*>::const_iterator it = _threadObjs.begin();it!=_threadObjs.end();++it) (*it)->Pause();
}
void InterruptableThread::PauseAllThreads(long ms){
    for(vector<InterruptableThread*>::const_iterator it = _threadObjs.begin();it!=_threadObjs.end();++it) (*it)->Pause(ms);
}
int InterruptableThread::GetRunningThreads(){
    return _running_threads.fetch_add(0,boost::memory_order_relaxed);
}
void InterruptableThread::ResumeAllThreads(){
    for(vector<InterruptableThread*>::const_iterator it = _threadObjs.begin();it!=_threadObjs.end();++it) (*it)->Resume();
}
vector<InterruptableThread*> InterruptableThread::GetAllThreads(){
    return _threadObjs;
}
InterruptableThread * InterruptableThread::GetThreadByID(boost::thread::id id){
    for(vector<InterruptableThread*>::const_iterator it = _threadObjs.begin();it!=_threadObjs.end();++it) if((*it)->GetThreadID()==id) return (*it);
    return NULL;
}

bool InterruptableThread::GetInterruptionEnabled(){
    return _interruption_enabled;
}
void InterruptableThread::SetInterruptionEnabled(bool enabled){
    _interruption_enabled = enabled;
}

bool InterruptableThread::Stop(bool force){
    if(_thisThread==NULL) return false;
    boost::mutex::scoped_lock lock(_mutex);

    if(force){ //stop on request, may damage process or application, LINUX ONLY
        pthread_kill(_thisThread->native_handle(),0);
        delete _thisThread;
        _thisThread = NULL;
        return true;
    }

    //tem uma requisicao, precisa ver qual eh predominante
    //stop eh supeior a todas
    //pause eh superior a pauseTimeout
    //pauseimeout eh superior a pausetimeout com tempo menor
    //portanto, nessa parada, basta sobreescrever os requests antigos

    _interrupt_request._target_state=ThreadState::Stopped;
    _interrupt_request._pause_timeout=0;
    _state = ThreadState::Stopping;
    _interrupt_request._active = true;
    _thisThread->interrupt();

    return true;
}

bool InterruptableThread::Pause(){
    if(_thisThread==NULL) return false;
    boost::mutex::scoped_lock lock(_mutex);
    if(!_interrupt_request._active || (_interrupt_request._target_state != ThreadState::Stopped)){
        _interrupt_request._target_state=ThreadState::Paused;
        _interrupt_request._pause_timeout=0;
        _state = ThreadState::Pausing;
        _interrupt_request._active = true;
        _thisThread->interrupt();
        return true;
    }
    return false;
}

bool InterruptableThread::Pause(long ms){
    if(_thisThread==NULL) return false;
    boost::mutex::scoped_lock lock(_mutex);
    if(!_interrupt_request._active || (_interrupt_request._target_state == ThreadState::PausedTimeout && _interrupt_request._pause_timeout < ms)){
        _interrupt_request._target_state=ThreadState::PausedTimeout;
        _interrupt_request._pause_timeout=ms;
        _state = ThreadState::PausingTimeout;
        _interrupt_request._active = true;
        _thisThread->interrupt();
        return true;
    }
    return false;
}

void InterruptableThread::_run(){
    _running_threads.fetch_add(1,boost::memory_order_relaxed);
    _interrupted = false;
    Start();
    while(true){
        try{
            if(_interruption_enabled) boost::this_thread::interruption_point();
            Update();
        }catch(boost::thread_interrupted&){
_run_interrupt:

            _interrupted = true;

            if(_interrupt_request._active){ //stopped by request

                boost::mutex::scoped_lock lock(_mutex);

                _state = _interrupt_request._target_state;
                switch(_interrupt_request._target_state){
                    case Paused:
                        try{
                            _cv.wait(lock); //case of reinterruption of higher level, shouldnt occour to wait locks
                        }catch(boost::thread_interrupted&){
                            goto _run_interrupt;
                        }
                    break;
                    case PausedTimeout:
                        try{

                            _cv.timed_wait(lock,boost::posix_time::milliseconds(_interrupt_request._pause_timeout));
                        }catch(boost::thread_interrupted&){
                            goto _run_interrupt;
                        }
                    break;
                    case Stopped:
                        _threadObjs.erase(std::remove(_threadObjs.begin(), _threadObjs.end(), this), _threadObjs.end()); //remove this from thread obj list
                        _interrupt_request._active = false;
                        _interrupt_request._target_state=ThreadState::Running;
                        _interrupt_request._pause_timeout=0;
                        _thisThread->detach();
                        _running_threads.fetch_sub(1,boost::memory_order_release);
                        delete _thisThread;
                        _thisThread = NULL;
                        return;
                    break;
                    default: //does nothing for other states, shouldnt be here also
                    break;
                }
            }else{
                //thread broke
            }

            _interrupted = false;
            _interrupt_request._active = false;
            _interrupt_request._target_state=ThreadState::Running;
            _interrupt_request._pause_timeout=0;
        }//try/catch
    }//while
}

InterruptableThread::InterruptableThread(){}
InterruptableThread::~InterruptableThread(){
    if(_thisThread!=NULL){
        Stop(true);
    }
}

bool InterruptableThread::Launch(){
    if(_thisThread!=NULL) return false;
    if(!CanLaunch()) return false;
    _thisThread = new boost::thread(boost::bind(&InterruptableThread::_run, this));
    InterruptableThread::_threadObjs.push_back(this);
    return true;
}

void InterruptableThread::Resume(){
    boost::mutex::scoped_lock lock(_mutex);
    _cv.notify_one();
}
*/
#endif // INTERRUPTABLETHREAD_INCLUDED
