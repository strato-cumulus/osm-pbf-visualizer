#ifndef OSM_UNPACK__BASES_H_DEFINED
#define OSM_UNPACK__BASES_H_DEFINED

namespace osm_unpack
{

template<typename T, template<typename> class Iterable>
class Iterator
{
protected:

    typename Iterable<T>::const_iterator it_;

public:

    Iterator(const Iterator<T, Iterable> & it);
    Iterator(typename Iterable<T>::const_iterator & it);

    virtual bool operator==(const typename Iterable<T>::const_iterator & other) const;
    virtual bool operator==(const Iterator<T, Iterable>& other) const;
    virtual bool operator!=(const typename Iterable<T>::const_iterator & other) const;
    virtual bool operator!=(const Iterator<T, Iterable>& other) const;
    virtual const T& operator*();
    virtual const T& operator->();

};

template<typename T, template<typename> class Iterable>
inline Iterator<T, Iterable>::Iterator(const Iterator<T, Iterable> &it):
    it_(it.it_) {}

template <typename T, template<typename> class Iterable>
inline Iterator<T, Iterable>::Iterator(typename Iterable<T>::const_iterator &it):
    it_(it) {}

template<typename T, template<typename> class Iterable>
inline bool Iterator<T, Iterable>::operator==(const typename Iterable<T>::const_iterator & other) const
{
    return other == this->it_;
}

template<typename T, template<typename> class Iterable>
inline bool Iterator<T, Iterable>::operator==(const Iterator<T, Iterable> & other) const
{
    return other == this->it_;
}

template<typename T, template<typename> class Iterable>
inline bool Iterator<T, Iterable>::operator!=(const typename Iterable<T>::const_iterator &other) const
{
    return !(*this == other);
}

template<typename T, template<typename> class Iterable>
inline bool Iterator<T, Iterable>::operator!=(const Iterator<T, Iterable> &other) const
{
    return !(*this == other);
}

template <typename T, template <typename> class Iterable>
inline const T &Iterator<T, Iterable>::operator*()
{
    return *this->it_;
}

template<typename T, template<typename> class Iterable>
inline const T& Iterator<T, Iterable>::operator->()
{
    return *this->it_;
}

}

#endif