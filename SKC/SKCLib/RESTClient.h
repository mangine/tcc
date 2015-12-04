#ifndef RESTCLIENT_H_
#define RESTCLIENT_H_

#include <string.h>
#include <boost/lockfree/queue.hpp>
#include <cpprest/http_client.h>
#include <boost/static_assert.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "InterruptableThread.h"
#include "IRestRequest.h"
#include "RESTQueue.h"

using namespace std;

namespace skc{

class RESTClient : public InterruptableThread {
private:
    const string _host;
	RESTQueue * _queue = NULL;
	pplx::task<void> _send_request_task(IRestRequest * r);
    static boost::atomic<int> _instances;
public:
    RESTClient();
	RESTClient(RESTQueue * queue);
	~RESTClient();
	RESTQueue * GetQueue(){ return _queue;}

	bool CanLaunch();
	void Update();
	void Start();
	void OnPause();
	void OnResume();
};

}

/*
namespace skc{


class RESTClient{

private:
    const string _host;
	boost::thread * thClient = NULL;
	RESTQueue * _queue = NULL;

	bool _enabled = false;
	bool _run = true;


	void _Run();
	pplx::task<void> _send_request_task(IRestRequest * r);
public:

	RESTClient();
	RESTClient(RESTQueue * queue);
	~RESTClient();
	bool Start();
	void Stop();
	void SetEnabled(bool enable);
	bool GetEnabled();
	RESTQueue * GetQueue(){ return _queue;}

};

};*/
#endif
