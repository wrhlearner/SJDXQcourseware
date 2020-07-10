////////////////////////////////////////////////////////////////////////////////
#pragma once
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cstdint>

#include <memory>
#include <any>
#include <vector>
#include <map>
#include <stack>
#include <array>
#include <string>

//types ***************************************************************************************************************************
typedef unsigned char  uchar;

//command ***************************************************************************************************************************

/*
class ICommandParameter
{
};
*/

class ICommandBase
{
public:
	//virtual void SetParameter(const std::shared_ptr<ICommandParameter>& param) = 0;
	virtual void SetParameter(const std::any& param) = 0;
	virtual void Exec() = 0;
/*
virtual std::shared_ptr<ICommandBase> get_Undo() = 0;
virtual std::shared_ptr<ICommandBase> get_Redo() = 0;
//use make_shared, then the virtual destructor is not necessary. (RAII)
*/
};

class CommandManager
{
public:
/*
...
*/
private:
	std::vector<std::shared_ptr<ICommandBase>> m_vecUndo;
	std::vector<std::shared_ptr<ICommandBase>> m_vecRedo;
};

//Notifications ***************************************************************************************************************************
//MVVM模式中有两个重要的通知
//当model内的数据发生改变，要向viewmodel发出通知
//viewmodel内的数据发生改变，或绑定到view的属性发生改变，也要发出通知
//在当前应用场景下，只有属性改变会发出通知

template <class T>
class NotificationImpl
{
//实际上结合三个方法和一个std::shared_ptr指针做了一个接受通知的接收器
//术语为sink，或callback
public:
	void Clear() throw()
	{
		m_array.clear();
	}
	void AddNotification(const std::shared_ptr<T>& p)
	{
		m_array.push_back(p);
	}
	void RemoveNotification(const std::shared_ptr<T>& p) throw()
	{
		auto iter(m_array.begin());
		for( ; iter != m_array.end(); ++ iter ) {
			if( (*iter).get() == p.get() ) {
				m_array.erase(iter);
				return ;
			}
		}
	}

protected:
	std::vector<std::shared_ptr<T>> m_array;
};

//通过一个纯虚类做通知接口
//对通知进行标识
//有两种标识方法：
//    1.整型数标识
//        比较快
//        但是需要在common层定义整数的标志
//    2.字符串比较标识
//        比较慢
//        解耦合比较好
class IPropertyNotification
{
public:
	virtual void OnPropertyChanged(const std::string& str) = 0;
	//virtual void OnPropertyChanged(unsigned int uPropertyID) = 0;
};

//命令通知
//需要知道每一条命令发出后是成功还是失败的
class ICommandNotification
{
public:
	virtual void OnCommandComplete(const std::string& str, bool bOK) = 0;
	//virtual void OnCommandComplete(unsigned int uCommandID, bool bOK) = 0;
};

template <class T>
class Proxy_PropertyNotification : public NotificationImpl<IPropertyNotification>
{
public:
	void AddPropertyNotification(const std::shared_ptr<IPropertyNotification>& p)
	{
		AddNotification(p);
	}
	void RemovePropertyNotification(const std::shared_ptr<IPropertyNotification>& p) throw()
	{
		RemoveNotification(p);
	}
	void Fire_OnPropertyChanged(const std::string& str)
  //触发
  //使用for循环
  //例如，当发生触发时，如果当前存在多个窗口，则每个窗口都可以通过for循环得到改变
	{
		auto iter(m_array.begin());
		for( ; iter != m_array.end(); ++ iter ) {
			(*iter)->OnPropertyChanged(str);
		}
	}
};
template <class T>
class Proxy_CommandNotification : public NotificationImpl<ICommandNotification>
{
public:
	void AddCommandNotification(const std::shared_ptr<ICommandNotification>& p)
	{
		AddNotification(p);
	}
	void RemoveCommandNotification(const std::shared_ptr<ICommandNotification>& p) throw()
	{
		RemoveNotification(p);
	}
	void Fire_OnCommandComplete(const std::string& str, bool bOK)
	{
		auto iter(m_array.begin());
		for( ; iter != m_array.end(); ++ iter ) {
			(*iter)->OnCommandComplete(str, bOK);
		}
	}
};

//state machine ***************************************************************************************************************************
//状态机一般用于window层，也可用于app层，实际情况实际分析
class IStateBase
{
public:
	virtual int32_t Process(uint32_t uEvent, const std::any& param) = 0;
};

class StateManager
{
public:
	void Add(int32_t iState, const std::shared_ptr<IStateBase>& spState)
	{
		m_map.insert(std::pair<int32_t, std::shared_ptr<IStateBase>>(iState, spState));
	}
	void SetStartState(int32_t iStartState) throw()
	{
		m_iCurrentState = iStartState;
	}
	void Process(uint32_t uEvent, const std::any& param)
	{
		auto iter(m_map.find(m_iCurrentState));
		if( iter != m_map.end() )
			m_iCurrentState = iter->second->Process(uEvent, param);
	}
	int32_t GetCurrentState() const throw()
	{
		return m_iCurrentState;
	}

private:
	int32_t m_iCurrentState;
  //在cstdint中定义
	std::map<int32_t, std::shared_ptr<IStateBase>> m_map;
  //map是红黑树
};


/***********************************************************************************************************************************/
//初步改造：使用lambda表达式和std function进行实现
/*
# command:

provider class: std::function<bool(...)> get_XXXCommand();
binding class: void set_XXXCommand(std::function<bool(...)>&& cmd);

# property notification

// [](uint32_t)
typedef std::function<void(uint32_t)>  PropertyNotification;

class PropertyTrigger
{
public:
	PropertyTrigger() noexcept;
	PropertyTrigger(const PropertyTrigger&) = delete;
	PropertyTrigger& operator=(const PropertyTrigger&) = delete;
	~PropertyTrigger() noexcept;

	void ClearNotifications() noexcept;
	void AddNotification(PropertyNotification&& pn);

	void Fire(uint32_t uID);

private:
	std::vector<PropertyNotification> m_vecPN;
};

# state machine

typedef std::funtion<int32_t(uint32_t)>  StateFunc;
provider class: StateFunc get_XXXStateFunc();

class StateManager
{
public:
	StateManager() noexcept;
	StateManager(const StateManager&) = delete;
	StateManager& operator=(const StateManager&) = delete;
	~StateManager() noexcept;

	void Add(int32_t iState, StateFunc&& sf);
	void SetStartState(int32_t iStartState) noexcept;
	void Process(uint32_t uEvent);
	int32_t GetCurrentState() const noexcept;

private:
	int32_t m_iCurrentState;
	std::map<int32_t, StateFunc> m_map;
};
*/

////////////////////////////////////////////////////////////////////////////////
