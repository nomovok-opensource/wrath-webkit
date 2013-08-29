#ifndef _HIERARCHYOfHandlesOfWRATH_
#define _HIERARCHYOfHandlesOfWRATH_

#include "PaintInfoOfWRATH.h"

namespace WebCore {

  /*
    T=the key type, must provide a connect_dtor method to connect to a signal that fires at dtor
    S=the type to which the handle associates (almost always RenderObject as well).

    In most circumstances, T and S are the same and their use within a HierarchyOfHandlesOfWRATH
    refers to the same object.
   */
template<typename T, typename S=RenderObject>
class HierarchyOfHandlesOfWRATH:boost::noncopyable
{
private:

  class per_object
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandleT<S> *m_data;
    boost::signals2::connection m_connection;
    std::vector<boost::signals2::connection> m_additional_connections;
  };

  typedef std::map<T*, per_object> map_type;
  typedef typename map_type::iterator iterator;
  typedef typename map_type::value_type value_type;
  

public:
  /*
    fires a signal whenever an object is add
   */
  typedef typename boost::signals2::signal<void (T*) > signal_t;
  typedef typename signal_t::slot_type slot_type;
  typedef boost::signals2::connection connection;

  virtual
  ~HierarchyOfHandlesOfWRATH() 
  {
    clear();
  }

  PaintedWidgetsOfWRATHHandleT<S>&
  getHandle(T *q)
  {
    return *getEntry(q).m_data;
  }

  void
  removeObject(T *q)
  {
    iterator iter;
    
    iter=m_per_object.find(q);
    if(iter!=m_per_object.end()) {
      deleteEntryDatum(iter);
      m_per_object.erase(iter);
    }
  }

  /*
    pass a connect that you wish to be disconnected
    when the named object is removed.
   */
  void
  noteConnection(T *q, boost::signals2::connection con)
  {
    getEntry(q).m_additional_connections.push_back(con);
  }

  /*
    signal fired whenever an entry is added
   */
  connection
  connectOnAddEntry(const slot_type &slot) 
  {
    return m_onNewEntry.connect(slot);
  }
  
  void
  clear(void)
  {
    for(iterator iter=m_per_object.begin(),
          end=m_per_object.end(); iter!=end; ++iter)
      {
        iter->second.m_connection.disconnect();
        deleteEntryDatum(iter);
      }
    m_per_object.clear();
  }

  void
  hideEachObject(void)
  {
    for(iterator iter=m_per_object.begin(),
          end=m_per_object.end(); iter!=end; ++iter)
      {
        iter->second.m_data->visible(false);
      }
  }


  template<typename F>
  void
  removeHandlesIf(const F &functor)
  {
    iterator iter(m_per_object.begin());
    iterator end(m_per_object.end());

    while(iter!=end)
      {
        if(functor(*iter->second.m_data))
          {
            iterator remove_iter(iter);
            ++iter;

            deleteEntryDatum(remove_iter);
            m_per_object.erase(remove_iter);
          }
        else
          {
            ++iter;
          }
      }
  }

  void
  removeNonVisibleHandles(void)
  {
    removeHandlesIf(!boost::bind(&PaintedWidgetsOfWRATHHandleT<S>::visible, _1) );
  }
  

private:

  void
  deleteEntryDatum(iterator iter)
  {
    iter->second.m_connection.disconnect();
    for(unsigned int i=0, lasti=iter->second.m_additional_connections.size(); i<lasti; ++i) {
      iter->second.m_additional_connections[i].disconnect();
    }
    
    WRATHDelete(iter->second.m_data);
  }

  per_object&
  getEntry(T *q)
  {
    iterator iter;
    
    iter=m_per_object.find(q);

    if(iter==m_per_object.end()) {
      per_object temp;

      temp.m_data=WRATHNew PaintedWidgetsOfWRATHHandleT<S>();
      temp.m_connection=q->connect_dtor(boost::bind(&HierarchyOfHandlesOfWRATH::removeObject,
                                                    this, q));

      iter=m_per_object.insert(value_type(q, temp)).first;
      m_onNewEntry(q);
    }
    
    return iter->second;
  }
  
  signal_t m_onNewEntry;
  map_type m_per_object;
};

  


}

#endif
