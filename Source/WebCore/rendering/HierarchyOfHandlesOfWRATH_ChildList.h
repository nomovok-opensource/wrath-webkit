#ifndef _HIERARCHYOfHandlesOfWRATHChildList_
#define _HIERARCHYOfHandlesOfWRATHChildList_

#include "HierarchyOfHandlesOfWRATH.h"
#include "RenderObjectChildList.h"

namespace WebCore {

  template<typename T>
  class HierarchyOfHandlesOfWRATHChildList:public HierarchyOfHandlesOfWRATH<T>
  {
  public:
    HierarchyOfHandlesOfWRATHChildList(RenderObjectChildList *list):
      m_list(list)
    {
      if(m_list)
        {
          m_on_change=m_list->connect( boost::bind(&HierarchyOfHandlesOfWRATHChildList::onChange,
                                                   this, _1, _2));
        }
    }

    ~HierarchyOfHandlesOfWRATHChildList()
    {
      if(m_list) {
        m_on_change.disconnect();
      }
    }

  private:
    void
    onChange(enum RenderObjectChildList::MemberChange v, RenderObject *po)
    {
      WRATHassert(po==0 or dynamic_cast<T*>(po)!=NULL);
      T *o( static_cast<T*>(po));

      if(v==RenderObjectChildList::E_RemoveChild) {
        this->removeObject(o);
      }
      else if (v==RenderObjectChildList::E_Dtor) {
        this->clear();
        m_on_change.disconnect();
        m_list=0;
      }
    }

    RenderObjectChildList *m_list;
    RenderObjectChildList::connection m_on_change;

  };

}

#endif
