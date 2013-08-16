/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#ifndef SQLINTERNALS_P_H
#define SQLINTERNALS_P_H

#include <boost/mpl/fold.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/push_back.hpp>

/**
 * @file SqlInternals_p.h
 * Shared internals between the various Sql related templates.
 */

namespace Sql {

namespace detail {

/**
 * type wrapper to avoid instantiation of concrete types
 * @tparam T The type to wrap
 * @internal
 */
template <typename T> struct wrap {};

/** Empty type for eg. not yet specified parts of a query. */
struct missing {};

}

/**
 * Metafunctions for concatenating two MPL vectors.
 * @tparam V1 First vector
 * @tparam V2 Second vector
 * @returns V1 + V2
 */
template <typename V1, typename V2>
struct append : boost::mpl::fold<
    V2,
    V1,
    boost::mpl::push_back<boost::mpl::placeholders::_,boost::mpl::placeholders::_>
>
{};


namespace detail {

/**
 * Implementation details for the conditional compiler warning generator template below.
 * @internal
 */
template <bool B>
struct warning_helper {
    typedef unsigned int signed_or_unsigned_int;
};
template <>
struct warning_helper<true> {
    typedef signed int signed_or_unsigned_int;
};

}

/**
 * Triggers a conditional compiler warning.
 * @tparam Condition A MPL boolean meta-type, the warning if generated when evaluating to @c true.
 * @tparam Message A message to be included in the output. Needs to be a defined symbol.
 * @note requires sign comparison warnings being enabled.
 */
template <typename Condition, typename Message>
struct warning {
    static inline void print() {
        const typename detail::warning_helper<Condition::value>::signed_or_unsigned_int a = 0;
        const unsigned int b = 0;
        const bool c = (a != b);
        (void) c;
    }
};


}

#endif
