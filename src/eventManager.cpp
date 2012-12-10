#include "eventManager.h"
#include <glog/logging.h>
#include <iostream>
#include <algorithm>

EventManager::EventManager():
	idCount_(0),
	registeredCbks_{},
	eventQueue_{}
{
	
}

EventManager::~EventManager()
{
	
}

int EventManager::subscribe(const std::string eventName, const CallBkFunc lambdaFunc)
{
	LOG(INFO) << "Subscribing to event: " << eventName;
	struct Callback cbk;
	cbk.id = idCount_;
	cbk.lambdaFunc = lambdaFunc;
	registeredCbks_.insert( {eventName,cbk} );
	++idCount_;
	return cbk.id;	
}

bool EventManager::unsubscribe(const int id)
{
	for (auto it = registeredCbks_.begin(); it != registeredCbks_.end(); ++it) {
		if (it->second.id == id) {
			LOG(INFO) << "Unsubscribe listener with ID = " << id;
			registeredCbks_.erase(it);
			return true;
		}
	}
	return false;
}

void EventManager::sendEvent(const std::string eventName, const Arguments args)
{
		LOG(INFO) << "New event received: " << eventName;
		MyEvent ev = { eventName, args };
		eventQueue_.push(ev);
}

void EventManager::processEvents()
{
	
	
	if (!eventQueue_.empty()) {
		LOG(INFO) << "========== Start processing events in queue ==========";
		while (!eventQueue_.empty()) {
			auto it = registeredCbks_.equal_range(eventQueue_.front().eventName);
			LOG(INFO) << "Processing some events in queue: " << eventQueue_.front().eventName;
			for (auto it2 = it.first; it2 != it.second; ++it2) {
				LOG(INFO) << "Call a matching method for event: " << "\"" << eventQueue_.front().eventName << "\"";
				it2->second.lambdaFunc(it2->first, eventQueue_.front().args);
			}
			eventQueue_.pop();
		}
		LOG(INFO) << "================ All events processed ================";
	}


}


void EventManager::listListeners() const
{
	if (!registeredCbks_.empty()) {
		for (auto &event : registeredCbks_) {
			std::cout << "[ " << event.first << " ] -> " << event.second.id << std::endl;
			//TODO add function signature
		}		
	} else {
		LOG(INFO) << "Nothing registered in registeredCbks_ ==> OK";
	}
}


///////////////////////////////////////
//	Subscribable
///////////////////////////////////////

Subscribable::Subscribable()
{
	
}

Subscribable::~Subscribable()
{
	for (auto ev : subscribedEventList_) {		
		EventMgrFactory::getCurrentEvtMgr()->unsubscribe(ev.second);		
	}
	subscribedEventList_.clear();
}

bool Subscribable::subscribe(const std::string eventName, CallBkFunc lambdaFunc)
{
	auto it = subscribedEventList_.find(eventName);
	if (it != subscribedEventList_.end()) {
		LOG(WARNING) << "Registering an event already registered on that object: both will be unsubscribed";		
	}
	int id = EventMgrFactory::getCurrentEvtMgr()->subscribe(eventName, lambdaFunc);
	subscribedEventList_.insert( {eventName, id} );
	return true;
}

bool Subscribable::unsubscribe(const std::string eventName)
{
	auto range = subscribedEventList_.equal_range(eventName);
	if (range.first == subscribedEventList_.end() && range.second == subscribedEventList_.end()) {
		LOG(INFO) << "No events to unsubscribe";
		return false;
	} else {
		if (range.first != range.second) {
			LOG(WARNING) << "Unregistering more than one callback for event: " << eventName;
		}
		for_each(
			range.first,
			range.second,
			[](std::unordered_multimap<std::string, int>::value_type &ev){
				EventMgrFactory::getCurrentEvtMgr()->unsubscribe(ev.second);}
		);
		return true;
	}
}

///////////////////////////////////////
//	EventMgrFactory
///////////////////////////////////////
std::pair<std::string, std::shared_ptr<EventManager>> EventMgrFactory::currentEvtMgr_ = {};
std::unordered_map<std::string, std::shared_ptr<EventManager>> EventMgrFactory::EventMgrList_ = {};

EventMgrFactory::EventMgrFactory()
{
	
}

EventMgrFactory::~EventMgrFactory() {
	
}

bool EventMgrFactory::createEvtMgr(std::string name)
{
	if (EventMgrList_.find(name) == EventMgrList_.end()) {
		EventMgrList_[name] = std::make_shared<EventManager>();
		currentEvtMgr_ = {name, EventMgrList_[name]};
		return true;
	} else {
		LOG(WARNING) << "Trying to create an EventManager with the same name: " << name;
		return false;
	}
}

bool EventMgrFactory::deleteEvtMgr(std::string name)
{
	auto EvtMgr = EventMgrList_.find(name);
	if (EvtMgr == EventMgrList_.end()) {
		LOG(WARNING) << "Trying to delete an EventManager that doesn't exist: " << name;
		return false;
	} else {
		if (currentEvtMgr_.second == EvtMgr->second) {
			LOG(WARNING) << "Trying to delete the current EventManager";
			return false;
		}
		EventMgrList_.erase(EvtMgr);
		return true;
	}
}

bool EventMgrFactory::setCurrentEvtMgr(std::string name)
{
	auto EvtMgr = EventMgrList_.find(name);
	if (EvtMgr == EventMgrList_.end()) {
		LOG(WARNING) << "Trying to select a non-existent EventManager: " << name;
		return false;
	} else {
		currentEvtMgr_ = {EvtMgr->first, EvtMgr->second};
		return true;		
	} 
}

EventManager* EventMgrFactory::getCurrentEvtMgr()
{
	if (currentEvtMgr_ == std::pair<std::string, std::shared_ptr<EventManager>>()) {
		return nullptr;
	} else {
		return currentEvtMgr_.second.get();
	}
}

std::string EventMgrFactory::getCurrentEvtMgrName()
{
	if (currentEvtMgr_ == std::pair<std::string, std::shared_ptr<EventManager>>()) {
		return "";
	} else {
		return currentEvtMgr_.first;
	}
}
