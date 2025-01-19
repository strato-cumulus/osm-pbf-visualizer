#ifndef TRANSFORMERS_H_INCLUDED
#define TRANSFORMERS_H_INCLUDED

#include <functional>

#include "bases.h"

namespace osm_unpack {

template<typename T, template<typename> class Iterable, template<typename> class Function>
class StatefulIterator: public Iterator<T, Iterable>
{
    T value_;
    typename Iterable<T>::const_iterator end_;

    Function<T> increment_fn;

public:

    StatefulIterator(const StatefulIterator<T, Iterable, Function>& it);
    StatefulIterator(typename Iterable<T>::const_iterator & begin,
        typename Iterable<T>::const_iterator & end, const Function<T> & increment_fn);

    StatefulIterator<T, Iterable, Function>& operator++();
    StatefulIterator<T, Iterable, Function> operator++(int);
    virtual const T& operator*();
    virtual const T& operator->();
};

template<typename T, template<typename> class Iterable, template<typename> class Function>
class StatefulIterable
{
    typename Iterable<T>::const_iterator begin_;
    typename Iterable<T>::const_iterator end_;

    Function<T> increment_fn;

public:

    StatefulIterator<T, Iterable, Function> begin();
    StatefulIterator<T, Iterable, Function> end();

    StatefulIterable(const Iterable<T> & iterable, const Function<T> & increment_fn);
};

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline StatefulIterator<T, Iterable, Function> StatefulIterable<T, Iterable, Function>::begin()
{
    return StatefulIterator<T, Iterable, Function>(this->begin_, this->end_, this->increment_fn);
}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline StatefulIterator<T, Iterable, Function> StatefulIterable<T, Iterable, Function>::end()
{
    return StatefulIterator<T, Iterable, Function>(this->end_, this->end_, this->increment_fn);
}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline StatefulIterable<T, Iterable, Function>::StatefulIterable(const Iterable<T> & iterable, const Function<T> & increment_fn):
    begin_(iterable.begin()), end_(iterable.end()), increment_fn(increment_fn) {}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline StatefulIterator<T, Iterable, Function>::StatefulIterator(const StatefulIterator<T, Iterable, Function>& it):
    Iterator<T, Iterable>(it), value_(it.value_), end_(it.end_) {}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline StatefulIterator<T, Iterable, Function>::StatefulIterator(typename Iterable<T>::const_iterator &begin,
                                                       typename Iterable<T>::const_iterator &end, const Function<T> & increment_fn):
    Iterator<T, Iterable>(begin), value_(begin != end ? *begin : NULL), end_(end), increment_fn(increment_fn) {}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline StatefulIterator<T, Iterable, Function>& StatefulIterator<T, Iterable, Function>::operator++()
{
    value_ = increment_fn(value_, *++this->it_);
    return *this;
}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline StatefulIterator<T, Iterable, Function> StatefulIterator<T, Iterable, Function>::operator++(int)
{
    auto old_copy = StatefulIterator<T, Iterable, Function>(*this);
    if ( this->it_ != end_ ) {
        ++*this;
    }
    return old_copy;
}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline const T& StatefulIterator<T, Iterable, Function>::operator*()
{
    return this->value_;
}

template<typename T, template<typename> class Iterable, template<typename> class Function>
inline const T& StatefulIterator<T, Iterable, Function>::operator->()
{
    return this->value_;
}
}

#endif