#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <unordered_map>
#include <boost/any.hpp>
#include <memory>
#include <functional>
#include <queue>

typedef std::unordered_map<std::string, boost::any> Arguments;
typedef std::function<void(std::string eventName, Arguments args)> CallBkFunc;


class EventManager
{
	private:
	
	public:
		EventManager();
		~EventManager();

		int subscribe(const std::string eventName, const CallBkFunc lambdaFunc);
		bool unsubscribe(const int id);
		void sendEvent(const std::string eventName, const Arguments args=Arguments());
		
		void processEvents();
		void listListeners() const;
	private:
		int idCount_;
		struct Callback {
			int id;
			CallBkFunc lambdaFunc;
		};
		typedef std::unordered_multimap<std::string, Callback> ListRegisteredCbks;
		ListRegisteredCbks registeredCbks_;
		struct MyEvent {
			std::string eventName;
			Arguments args;
		};
		std::queue<MyEvent> eventQueue_;
};

class Subscribable
{
	public:
		Subscribable();
		~Subscribable();
		
	protected:
		bool subscribe(const std::string eventName, CallBkFunc lambdaFunc);
		bool unsubscribe(const std::string eventName);
		
	private:	
		std::unordered_multimap<std::string, int> subscribedEventList_;
	
};

class EventMgrFactory
{
	public:
		EventMgrFactory();
		~EventMgrFactory();
		
		static bool createEvtMgr(std::string name);
		static bool deleteEvtMgr(std::string name);
		static bool setCurrentEvtMgr(std::string name);
		static EventManager* getCurrentEvtMgr();
		static std::string getCurrentEvtMgrName();
	private:
		static std::unordered_map<std::string, std::shared_ptr<EventManager>> EventMgrList_;
		static std::pair<std::string, std::shared_ptr<EventManager>> currentEvtMgr_;
};


#endif
