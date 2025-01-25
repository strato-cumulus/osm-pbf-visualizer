#ifndef OSM_UNPACK__BASES_H_DEFINED
#define OSM_UNPACK__BASES_H_DEFINED

namespace osm_unpack
{

template<typename T, template<typename> class BaseIterator>
class Iterator
{
protected:

    BaseIterator<T> it_;

public:

    Iterator(const BaseIterator<T> & it);

    virtual bool operator==(const BaseIterator<T> & other) const;
    virtual bool operator!=(const BaseIterator<T> & other) const;
    virtual const T& operator*();
    virtual const T& operator->();

};

template<typename T, template<typename> class BaseIterator>
inline Iterator<T, BaseIterator>::Iterator(const BaseIterator<T> &it):
    it_(it) {}

template<typename T, template<typename> class BaseIterator>
inline bool Iterator<T, BaseIterator>::operator==(const BaseIterator<T> & other) const
{
    return other == this->it_;
}

template<typename T, template<typename> class BaseIterator>
inline bool Iterator<T, BaseIterator>::operator!=(const BaseIterator<T> &other) const
{
    return !(*this == other);
}

template <typename T, template <typename> class BaseIterator>
inline const T &Iterator<T, BaseIterator>::operator*()
{
    return *this->it_;
}

template<typename T, template<typename> class BaseIterator>
inline const T& Iterator<T, BaseIterator>::operator->()
{
    return *this->it_;
}

}

#endif