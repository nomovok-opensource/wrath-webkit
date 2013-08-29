#ifndef Signal_H
#define Signal_H

#include "config.h"
#if USE(WRATH)
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

namespace WebCore {
template <typename T>
struct SignalTag {};

template <typename T>
class Signal {
public:
    typedef SignalTag<T> tag_t;
    typedef typename boost::signals2::signal<void (typename T::MemberChange) > signal_t;
    typedef typename signal_t::slot_type slot_type;
    typedef boost::signals2::connection connection;

    Signal() : m_sig() {}

    // Force copy-ctor to ignore copying.
    Signal(const Signal & s) : m_sig() {}

    boost::signals2::connection connect(const slot_type & s)
    {
        return m_sig.connect(s);
    }

    void operator()(typename T::MemberChange e) const
    {
        m_sig(e);
    }

    // A convenience function
    template <typename R>
    void setValueAndEmit(R & m, const R & v, typename T::MemberChange e)
    {
        if(v != m)
        {
            m = v;
            m_sig(e);
        }
    }

private:
    signal_t m_sig;
};
}
#define SET_VALUE_AND_EMIT(member, value, signalObject, signal) do { if ((member) != (value)) { (member) = (value);  (signalObject)((signal)); } } while (0)
#else
#define SET_VALUE_AND_EMIT(member, value, signalObject, signal) do { (member) = (value); } while (0)
#endif /* USE(WRATH) */
#endif /* Signal_H */
