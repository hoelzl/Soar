/////////////////////////////////////////////////////////////////
// EventManager class file.
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : October 2004
//
// This class manages a list of connections which are interested
// in being notified when a particular event occurs in the kernel.
//
/////////////////////////////////////////////////////////////////

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include "gSKI_Enumerations.h"

#ifdef _MSC_VER
#pragma warning (disable : 4702)  // warning C4702: unreachable code, need to disable for VS.NET 2003 due to STL "bug" in certain cases
#endif
#include <list>
#include <map>
#ifdef _MSC_VER
#pragma warning (default : 4702)
#endif

namespace sml {

class KernelSML ;
class Connection ;

// The list of connections interested in an event
typedef std::list< Connection* >		ConnectionList ;
typedef ConnectionList::iterator	ConnectionListIter ;

template<typename EventType>
class EventManager
{
protected:
	ConnectionList*	GetListeners(EventType eventID)
	{
		EventMapIter mapIter = m_EventMap.find(eventID) ;

		if (mapIter == m_EventMap.end())
			return NULL ;

		return mapIter->second ;
	}

public:
    // Mapping from the event to the list of connections listening to that event
    typedef std::map< EventType, ConnectionList* >	EventMap ;
    typedef typename EventMap::iterator						EventMapIter ;
protected:
	// Map from event id to list of connections listening to that event
	EventMap		m_EventMap ;
public:
	virtual ~EventManager()
	{
		// This is interesting.  We can't call clear in the destructor for the event manager because it
		// calls virtual methods, which is apparently illegal (I get a "pure virtual method" error from Visual Studio).
		// So we'll need to either move this to the derived classes destructors or call it explicitly before we destroy the manager.
		// Clear() ;
	}

	// Clear the map and release all listeners
	virtual void Clear()
	{
		for (EventMapIter mapIter = m_EventMap.begin() ; mapIter != m_EventMap.end() ; mapIter++)
		{
			EventType eventID = mapIter->first ;
			ConnectionList* pList = mapIter->second ;

			// Can't walk through with a normal iterator because we're deleting
			// the items from the list as we go.  We do all this so that ultimately
			// the listener into the kernel will be removed (if RemoveListener for the specific
			// event class wishes to do so).
			while (pList->size() > 0)
			{
				Connection* pConnection = pList->front() ;
				RemoveListener(eventID, pConnection) ;
			}

			delete pList ;
		}

		m_EventMap.clear() ;
	}

	// Returns true if this is the first connection listening for this event
	virtual bool BaseAddListener(EventType eventID, Connection* pConnection)
	{
		EventMapIter mapIter = m_EventMap.find(eventID) ;

		ConnectionList* pList = NULL ;

		// Either create a new list or retrieve the existing list of listeners
		if (mapIter == m_EventMap.end())
		{
			pList = new ConnectionList() ;
			m_EventMap[eventID] = pList ;
		}
		else
		{
			pList = mapIter->second ;
		}

		pList->push_back(pConnection) ;

		// Return true if this is the first listener for this event
		return (pList->size() == 1) ;
	}

	// Returns true if this is the first connection listening for this event
	// (Generally calls BaseAddListener and then registers with the kernel for this event)
	virtual bool AddListener(EventType eventID, Connection* pConnection) = 0 ;

	// Returns true if just removed the last listener
	virtual bool BaseRemoveListener(EventType eventID, Connection* pConnection)
	{
		ConnectionList* pList = GetListeners(eventID) ;

		// We have no record of anyone listening for this event
		// That's not an error -- it's OK to call this for all events in turn
		// to make sure a connection is removed completely.
		if (pList == NULL || pList->size() == 0)
			return false ;

		pList->remove(pConnection) ;

		// Return true if the list is now empty
		return pList->empty() ;
	}

	// Returns true if just removed the last listener
	// (Generally calls BaseRemoveListener and then unregisters with the kernel for this event)
	virtual bool RemoveListener(EventType eventID, Connection* pConnection) = 0 ;

	// Remove all listeners that this connection has
	virtual void RemoveAllListeners(Connection* pConnection)
	{
		// Remove any listeners for this connection
		// We do this for all possible events even though only some will
		// be valid for this particular event manager.
		// We could make this more efficient by defining the set for each handler
		// but the cost of removing all should be minimal as this is a rare event.
		for (int i = 1 ; i < gSKIEVENT_LAST ; i++)
		{
			EventType id = (EventType)i ;
			RemoveListener(id, pConnection) ;
		}
	}

	virtual ConnectionListIter	GetBegin(EventType eventID)
	{
		ConnectionList* pList = GetListeners(eventID) ;

		// If nobody is listening return NULL.
		// Key is that this must match the value returned by GetEnd()
		// in the same situation.
		if (!pList)
			return NULL ;

		return pList->begin() ;
	}

	virtual ConnectionListIter  GetEnd(EventType eventID)
	{
		ConnectionList* pList = GetListeners(eventID) ;

		if (!pList)
			return NULL ;

		return pList->end() ;
	}
} ;

} // End of namespace

#endif	// End of header ifdef
