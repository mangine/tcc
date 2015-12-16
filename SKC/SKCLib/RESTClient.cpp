#include "skc.h"
#include "RESTClient.h"

#ifdef __EDISON__
#include "EdisonHelper.h"
#endif // __EDISON__

#include <cpprest/filestream.h>
#include <boost/function.hpp>

#include <cpprest/json.h>

namespace skc{

boost::atomic<int> RESTClient::_instances(0);

void RESTClient::Start(){
    _instances.fetch_add(1,boost::memory_order_relaxed);
}

void RESTClient::OnPause(){
    int instances = _instances.fetch_sub(1,boost::memory_order_relaxed);
    if(instances==0){
        #ifdef __EDISON__
        EdisonHelper::DisableWiFi();
        #endif // __EDISON__
    }
}

void RESTClient::OnResume(){
    _instances.fetch_add(1,boost::memory_order_relaxed);
    #ifdef __EDISON__
    EdisonHelper::EnableWiFi();
    #endif // __EDISON__
}

bool RESTClient::CanLaunch(){
    return (_queue!=NULL);
}

RESTClient::RESTClient(){
	_queue = new RESTQueue();
}

RESTClient::RESTClient(RESTQueue * queue){
	_queue = queue;
}

RESTClient::~RESTClient(){
}


pplx::task<void> RESTClient::_send_request_task(IRestRequest * r)
{
    if(r==NULL) return pplx::task_from_result();

    cout << "[RESTClient] About to send: " << endl << r->ToString() << endl;
    cout << "-Sending data to: " << r->GetUrl() << endl;

    web::http::client::http_client client(r->GetUrl());

    cout << "-HAS CALLBACK=" << (r->hasCallback()) << endl;


    if (r->hasCallback()){ //ARRUMA r, PARA NAO SUMIR DA MEMORIA
         try
         {
            return client.request(r->GetMethod(), r->GetPath(), r->ToString(), U("application/json")).then([=](web::http::http_response response){
                cout << "-ENDED REQUEST. Calling function: " << (r->hasCallback()) << endl;
                r->InvokeCallback(response);
                delete r;
            });/*.then([=](pplx::task<web::json::value> t)
            {
                handle_exception(t, U("field"));
                return pplx::task_from_result();
                //return
            });*/
        }
        catch (web::http::http_exception const & e) //possivelmente sem conexao
        {
            cout << "-ERROR: " << e.what();
            delete r;
        }
    }else{
        try{
            client.request(r->GetMethod(), r->GetPath(), r->ToString(), U("application/json"));/*.then([=](pplx::task<json::value> t)
            {
                handle_exception(t, U("field"));
                return pplx::task_from_result();
            });*/
            delete r;
            return pplx::task_from_result();
        }
        catch (web::http::http_exception const & e) //possivelmente sem conexao
        {
            cout << "-ERROR: " << e.what();
            delete r;
            return pplx::task_from_result();
        }
    }

    //remove warning apenas, mas nunca chega aqui
    return pplx::task_from_result();

}

pplx::task<web::json::value> RESTClient::handle_exception(pplx::task<web::json::value>& t, const utility::string_t& field_name)
{
    try
    {
        t.get();
    }
    catch(const std::exception& ex)
    {
        web::json::value error_json = web::json::value::object();
        error_json[field_name] = web::json::value::object();
        error_json[field_name]["error"] = web::json::value::string(utility::conversions::to_string_t(ex.what()));
        return pplx::task_from_result<web::json::value>(error_json);
    }

    return t;
}


void RESTClient::Update(){
    ///TODO testar condition variable
    if (_queue->empty()){
        boost::mutex::scoped_lock lock(*GetMutex());
        _queue->GetNotEmptyConditionVariable()->wait(lock);
    }
    cout << "NET QUEUE SIZE: " << _queue->size();
    //preserva valor, pois pode ser usado depois num callback
    _queue->consume_preserve(boost::bind(&RESTClient::_send_request_task,this,_1)); //_queue->consume(boost::bind(&RESTClient::_send_request_task,*this,_1));
}


/*

bool RESTClient::Start(){
	if (thClient != NULL) return false;
	if (_queue == NULL) return false;
	_enabled = true;
	_run = true;
	thClient = new boost::thread(boost::bind(&RESTClient::_Run, this)); //inicia thread da propria classe
	return true;
}

void RESTClient::Stop(){
	_enabled = false;
	_run = false;
	thClient->join(); //pode ser join, mas ai tem espera //pode ser detach, mas tenho medo de vazar memoria
	delete thClient;
	thClient = NULL;
}


void RESTClient::SetEnabled(bool enable){
	_enabled = enable;
}


bool RESTClient::GetEnabled(){
	return _enabled;
}

RESTClient::RESTClient(){
	_queue = new RESTQueue();
}

RESTClient::RESTClient(RESTQueue * queue){
	_queue = queue;
}

RESTClient::~RESTClient(){
	Stop();
}


pplx::task<void> RESTClient::_send_request_task(IRestRequest * r)
{
    if(r==NULL) return pplx::task_from_result();

    cout << "[RESTClient] About to send: " << endl << r->ToString() << endl;
    cout << "-Sending data to: " << r->GetUrl() << endl;

    web::http::client::http_client client(r->GetUrl());

    cout << "-HAS CALLBACK=" << (r->hasCallback()) << endl;


    if (r->hasCallback()){ //ARRUMA r, PARA NAO SUMIR DA MEMORIA
         try
         {
            return client.request(r->GetMethod(), r->GetPath(), r->ToString(), U("application/json")).then([=](web::http::http_response response){
                cout << "-ENDED REQUEST. Calling function: " << (r->hasCallback()) << endl;
                r->InvokeCallback(response);
                delete r;
            });
        }
        catch (web::http::http_exception const & e) //possivelmente sem conexao
        {
            cout << "-ERROR: " << e.what();
            delete r;
        }
    }else{
        try{
            client.request(r->GetMethod(), r->GetPath(), r->ToString(), U("application/json"));
            delete r;
            return pplx::task_from_result();
        }
        catch (web::http::http_exception const & e) //possivelmente sem conexao
        {
            cout << "-ERROR: " << e.what();
            delete r;
            return pplx::task_from_result();
        }
    }

    //remove warning apenas, mas nunca chega aqui
    return pplx::task_from_result();

}

void RESTClient::_Run(){
	while (_run){
		while (!_enabled);
		while (_enabled){
			while (_queue->empty());
			cout << "Item count: " << _queue->size();
			//preserva valor, pois pode ser usado depois num callback
			_queue->consume_preserve(boost::bind(&RESTClient::_send_request_task,this,_1)); //_queue->consume(boost::bind(&RESTClient::_send_request_task,*this,_1));
		}
	}
}
*/
};

