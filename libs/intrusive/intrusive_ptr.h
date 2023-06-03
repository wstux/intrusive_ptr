/*
 * The MIT License
 *
 * Copyright 2023 Chistyakov Alexander.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _INTRUSIVE_INTRUSIVE_PTR_H
#define _INTRUSIVE_INTRUSIVE_PTR_H

#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>

#define INIT_INTRUSIVE_PTR                                                      \
    private:                                                                    \
        unsigned int m_ref_counter = 0;                                         \
        template<typename T> friend void ::wstux::intrusive_ptr_add_ref(T* ptr);\
        template<typename T> friend void ::wstux::intrusive_ptr_release(T* ptr)

#define INIT_ATOMIC_INTRUSIVE_PTR                                               \
    private:                                                                    \
        std::atomic_size_t m_ref_counter = 0;                                   \
        template<typename T> friend void ::wstux::intrusive_ptr_add_ref(T* ptr);\
        template<typename T> friend void ::wstux::intrusive_ptr_release(T* ptr)

namespace wstux {
namespace details {

template<typename From, typename To>
struct __enable_if_convertible
{
    typedef typename std::enable_if<std::is_convertible<From, To>::value>::type type;
};

} // namespace details

template<typename T>
inline void intrusive_ptr_add_ref(T* ptr)
{
    ++(ptr->m_ref_counter);
}

template<typename T>
inline void intrusive_ptr_release(T* ptr)
{
    if (--ptr->m_ref_counter == 0) {
        delete ptr;
    }
}

template<typename T>
class intrusive_ptr
{
    typedef T* __ptr_type;

public:
    typedef T element_type;

    intrusive_ptr()
        : m_ptr(NULL)
    {}

    intrusive_ptr(element_type* ptr)
        : m_ptr(ptr)
    {
        if (m_ptr != NULL) {
            intrusive_ptr_add_ref(m_ptr);
        }
    }

    intrusive_ptr(const intrusive_ptr& rhs)
        : m_ptr(rhs.m_ptr)
    {
        if (m_ptr != NULL) {
            intrusive_ptr_add_ref(m_ptr);
        }
    }

    //template<typename U, typename = typename details::__enable_if_convertible<U, T>::type>
    template<typename U>
    intrusive_ptr(const intrusive_ptr<U>& rhs)//, typename details::__enable_if_convertible<U, T>::type* = 0)
        : m_ptr(rhs.get())
    {
        if (m_ptr != NULL) {
            intrusive_ptr_add_ref(m_ptr);
        }
    }

    intrusive_ptr(intrusive_ptr&& rhs)
        : m_ptr(rhs.m_ptr)
    {
        rhs.m_ptr = NULL;
    }

    //template<class U, typename = typename details::__enable_if_convertible<U, T>::type>
    template<class U>
    intrusive_ptr(intrusive_ptr<U>&& rhs)//, typename details::__enable_if_convertible<U, T>::type* = 0)
        : m_ptr(rhs.m_ptr)
    {
        rhs.m_ptr = NULL;
    }

    ~intrusive_ptr()
    {
        if (m_ptr != NULL) {
            intrusive_ptr_release(m_ptr);
        }
    }

    intrusive_ptr& operator=(const intrusive_ptr& rhs)
    {
        intrusive_ptr(rhs).swap(*this);
        return *this;
    }

    //template<typename U, typename = typename details::__enable_if_convertible<U, T>::type>
    template<class U>
    intrusive_ptr& operator=(const intrusive_ptr<U>& rhs)
    {
        intrusive_ptr(rhs).swap(*this);
        return *this;
    }

    intrusive_ptr& operator=(intrusive_ptr&& rhs)
    {
        intrusive_ptr(static_cast<intrusive_ptr&&>(rhs)).swap(*this);
        return *this;
    }

    //template<typename U, typename = typename details::__enable_if_convertible<U, T>::type>
    template<class U>
    intrusive_ptr& operator=(intrusive_ptr<U>&& rhs)
    {
        intrusive_ptr(static_cast<intrusive_ptr<U>&&>(rhs)).swap(*this);
        return *this;
    }

    intrusive_ptr& operator=(element_type* rhs)
    {
        intrusive_ptr(rhs).swap(*this);
        return *this;
    }

    element_type& operator*() const { assert(m_ptr != NULL); return *m_ptr; }

    element_type* operator->() const { assert(m_ptr != NULL); return m_ptr; }

    bool operator!() const { return (m_ptr == NULL); }

    explicit operator bool () const { return (m_ptr != NULL); }

    element_type* get() const { return m_ptr; }

    void swap(intrusive_ptr& rhs)
    {
        __ptr_type p_tmp = m_ptr;
        m_ptr = rhs.m_ptr;
        rhs.m_ptr = p_tmp;
    }

    void reset() { intrusive_ptr().swap(*this); }

    void reset(element_type* rhs) { intrusive_ptr(rhs).swap(*this); }

private:
    __ptr_type m_ptr;
};

template<typename T, typename U>
inline bool operator==(const intrusive_ptr<T>& lhs, const intrusive_ptr<U>& rhs)
{
    return lhs.get() == rhs.get();
}

template<typename T, typename U>
inline bool operator!=(const intrusive_ptr<T>& lhs, const intrusive_ptr<U>& rhs)
{
    return lhs.get() != rhs.get();
}

template<typename T, typename U>
inline bool operator==(const intrusive_ptr<T>& lhs, const U* rhs)
{
    return lhs.get() == rhs;
}

template<typename T, typename U>
inline bool operator!=(const intrusive_ptr<T>& lhs, const U* rhs)
{
    return lhs.get() != rhs;
}

template<typename T, typename U>
inline bool operator==(const T* lhs, const intrusive_ptr<U>& rhs)
{
    return lhs == rhs.get();
}

template<typename T, typename U>
inline bool operator!=(const T* lhs, const intrusive_ptr<U>& rhs)
{
    return lhs != rhs.get();
}

template<typename T, typename... TArgs>
inline intrusive_ptr<T> make_intrusive(TArgs&&... args)
{
    typedef typename std::remove_const<T>::type T_nc;

    try {
        return intrusive_ptr<T>(::new T_nc(std::forward<TArgs>(args)...));
    } catch (...) {
        throw;
    }
}

} // namespace wstux

namespace std {

template<typename T>
struct hash<::wstux::intrusive_ptr<T>>;

template<typename T>
struct hash<::wstux::intrusive_ptr<T>>
{
    size_t operator()(const ::wstux::intrusive_ptr<T>& ptr) const
    {
        return ::std::hash<T*>()(ptr.get());
    }
};

template<typename T>
void swap(::wstux::intrusive_ptr<T>& lhs, ::wstux::intrusive_ptr<T>& rhs)
{
    lhs.swap(rhs);
}

} // namespace std

#endif /* _INTRUSIVE_INTRUSIVE_PTR_H */
