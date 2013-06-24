#ifndef __KDTOOLSCORE__PIMPL_PTR_H__
#define __KDTOOLSCORE__PIMPL_PTR_H__

#include "kdtoolsglobal.h"

#ifndef DOXYGEN_RUN
namespace kdtools {
#endif

    template <typename T>
    class MAKEINCLUDES_EXPORT pimpl_ptr {
        KDAB_DISABLE_COPY( pimpl_ptr );
        T * d;
    public:
        pimpl_ptr() : d( new T ) {}
        explicit pimpl_ptr( T * t ) : d( t ) {}
        ~pimpl_ptr() { delete d; d = 0; }

        void swap( pimpl_ptr & other ) {
            T * const tmp = d;
            d = other.d;
            other.d = tmp;
        }

        T * get() { return d; }
        const T * get() const { return d; }

        T * operator->() { return get(); }
        const T * operator->() const { return get(); }

        T & operator*() { return *get(); }
        const T & operator*() const { return *get(); }

        KDAB_IMPLEMENT_SAFE_BOOL_OPERATOR( get() )
    };

    template <typename T>
    inline void swap( pimpl_ptr<T> & lhs, pimpl_ptr<T> & rhs ) {
        lhs.swap( rhs );
    }

    // these are not implemented, so's we can catch their use at
    // link-time. Leaving them undeclared would open up a comparison
    // via operator unspecified-bool-type().
    template <typename T, typename S>
    void operator==( const pimpl_ptr<T> &, const pimpl_ptr<S> & );
    template <typename T, typename S>
    void operator!=( const pimpl_ptr<T> &, const pimpl_ptr<S> & );

#ifndef DOXYGEN_RUN
} // namespace kdtools
#endif

#endif /* __KDTOOLSCORE__PIMPL_PTR_H__ */

