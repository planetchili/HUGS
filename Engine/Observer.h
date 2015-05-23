#pragma once
#include <vector>
#include <algorithm>

class Observer
{
public:
	virtual void OnNotify() = 0;
};

class Observable
{
public:
	inline void AddObserver( Observer& obs )
	{
		observers.push_back( &obs );
	}
	inline void RemoveObserver( Observer& obs )
	{
		std::remove( observers.begin(),observers.end(),&obs );
	}
	inline void Disable()
	{
		enabled = false;
	}
	inline void Enable()
	{
		enabled = true;
	}
	inline bool IsEnabled() const
	{
		return enabled;
	}
protected:
	inline void Notify()
	{
		for( Observer* obs : observers )
		{
			obs->OnNotify();
		}
	}
private:
	std::vector< Observer* > observers;
	bool enabled = true;
};