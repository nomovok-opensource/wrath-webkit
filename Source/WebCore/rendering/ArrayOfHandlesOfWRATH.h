#ifndef _ArrayOfHandlesOfWRATH_
#define _ArrayOfHandlesOfWRATH_

#include "PaintInfoOfWRATH.h"


namespace WebCore {
/*
  when you want a dynamic array of handles, well here you go:
 */

template<typename T=RenderObject>
class ArrayOfHandlesOfWRATH:boost::noncopyable
{
public:
  
  ~ArrayOfHandlesOfWRATH(void)
  {
    clear();
  }

  PaintedWidgetsOfWRATHHandleT<T>&
  operator[](unsigned int idx)
  {
    return *m_handles[idx];
  }

  unsigned int 
  size(void) const
  {
    return m_handles.size();
  }

  void
  clear(void)
  {
    resize(0);
  }

  void
  resize(unsigned int new_size)
  {
    unsigned int old_size(m_handles.size());

    for(unsigned int i=new_size; i<old_size; ++i)
      {
        WRATHDelete(m_handles[i]);
      }

    m_handles.resize(new_size);

    for(unsigned int i=old_size; i<new_size; ++i)
      {
        m_handles[i]=WRATHNew PaintedWidgetsOfWRATHHandleT<T>();
      }
    
  }

  void
  visible_each(bool v)
  {
    for(unsigned int i=0, endi=size(); i<endi; ++i)
      {
        this->operator[](i).visible(v);
      }
  }


private:
  std::vector< PaintedWidgetsOfWRATHHandleT<T>*> m_handles;
};


}


#endif
