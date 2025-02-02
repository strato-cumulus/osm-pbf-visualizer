#ifndef TRANSFORMERS_H_INCLUDED
#define TRANSFORMERS_H_INCLUDED

#include <functional>

#include "bases.h"

namespace osm_unpack {

template<typename T, template<typename U> class BaseIterator, template<typename V> class Function>
class StatefulIterator: public Iterator<T, BaseIterator>
{
    typename std::remove_const<T>::type value_;
    BaseIterator<T> end_;

    Function<T> increment_fn;

public:

    StatefulIterator(const StatefulIterator<T, BaseIterator, Function>& it);
    StatefulIterator(const BaseIterator<T> & begin,
        const BaseIterator<T> & end, const Function<T> & increment_fn);

    StatefulIterator<T, BaseIterator, Function>& operator++();
    StatefulIterator<T, BaseIterator, Function> operator++(int);
    virtual const T& operator*();
    virtual const T& operator->();
};

template<typename T, template<typename U> class BaseIterator, template<typename V> class Function>
inline StatefulIterator<T, BaseIterator, Function>::StatefulIterator(const StatefulIterator<T, BaseIterator, Function>& it):
    Iterator<T, BaseIterator>(it), value_(it.value_), end_(it.end_), increment_fn(it.increment_fn) {}

template<typename T, template<typename U> class BaseIterator, template<typename V> class Function>
inline StatefulIterator<T, BaseIterator, Function>::StatefulIterator(const BaseIterator<T> &begin,
    const BaseIterator<T> &end, const Function<T> &increment_fn):
    Iterator<T, BaseIterator>(begin), value_(begin == end ? NULL : *begin), end_(end), increment_fn(increment_fn)
{}

template<typename T, template<typename U> class BaseIterator, template<typename V> class Function>
inline StatefulIterator<T, BaseIterator, Function> &StatefulIterator<T, BaseIterator, Function>::operator++()
{
    ++this->it_;
    if ( this->it_ != this->end_ ) {
        value_ = increment_fn(value_, *this->it_);
    }
    return *this;
}

template<typename T, template<typename U> class BaseIterator, template<typename V> class Function>
inline StatefulIterator<T, BaseIterator, Function> StatefulIterator<T, BaseIterator, Function>::operator++(int)
{
    auto old_copy = StatefulIterator<T, BaseIterator, Function>(*this);
    if ( this->it_ != end_ ) {
        ++*this;
    }
    return old_copy;
}

template<typename T, template<typename U> class BaseIterator, template<typename V> class Function>
inline const T& StatefulIterator<T, BaseIterator, Function>::operator*()
{
    return this->value_;
}

template<typename T, template<typename U> class BaseIterator, template<typename V> class Function>
inline const T& StatefulIterator<T, BaseIterator, Function>::operator->()
{
    return this->value_;
}
}

#endif