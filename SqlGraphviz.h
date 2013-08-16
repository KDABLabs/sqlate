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
#ifndef SQLGRAPHVIZ_H
#define SQLGRAPHVIZ_H

#include "SqlSchema_p.h"
#include "SqlInternals_p.h"
#include "SqlCreateTable.h"

#include <QDateTime>
#include <QStringBuilder>
#include <QStringList>
#include <QUuid>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/size.hpp>

/**
 * @file SqlGraphviz.h
 * Dump the database schema into a dot file for visualization.
 */

namespace Sql {

namespace detail {

/**
 * Helper class to dump a single column to dot format.
 * @internal
 * @tparam C The column type
 */
struct column_dot_creator
{
    column_dot_creator( QStringList &c, QStringList &edges ) : cols( c ), m_edges( edges ) {}

    template<typename C> void operator()( wrap<C> )
    {
        const QString colStmt =
            QLatin1Literal( "<TR><TD ALIGN=\"LEFT\" PORT=\"" ) % C::sqlName() % QLatin1Literal( "\"><I>" )
            % sqlType( wrap<typename C::type>(), C::size )
            % QLatin1Literal( "</I>: " )
            % C::sqlName() % QLatin1Literal( "</TD></TR>" );
        foreignKeyEdge<C>( typename C::hasForeignKey() );
        cols.push_back( colStmt );
    }

    /**
     * Creates a graph edge for foreign key references.
     * @tparam C The column type.
     */
    template<typename C> void foreignKeyEdge( boost::mpl::false_ ) {}
    template<typename C> void foreignKeyEdge( boost::mpl::true_ )
    {
        QString edge = C::table::tableName() % QLatin1Literal( ":" ) % C::sqlName()
            % QLatin1Literal( " -> " )
            % C::referenced_column::table::tableName() % QLatin1Literal( ":" ) % C::referenced_column::sqlName()
            % QLatin1Literal( "[label=\"n:1\"];\n" );
        m_edges.push_back( edge );
    }

    QStringList &cols;
    QStringList &m_edges;
};

/**
 * Helper class to create table nodes in dot format
 * @internal
 * @tparam T The table type.
 */
struct table_dot_creator
{
    table_dot_creator( QStringList & nodes, QStringList &edges ) : m_nodes( nodes ), m_edges( edges ) {};

    template <typename T>
    void operator() ( wrap<T> )
    {
        if ( T::is_lookup_table::value ) {
            makeNode<T>( QLatin1String( "lightyellow" ) );
        } else if ( !T::is_relation::value ) {
            makeNode<T>( QLatin1String( "lightsteelblue" ) );
        }
        if ( T::is_relation::value && boost::mpl::size<typename T::columns>::value > 2 )
            makeNode<T>( QLatin1String( "lightgray" ) );
        else
            makeRelationEdge<T>( typename T::is_relation() );
    }

    template <typename T>
    void makeNode( const QString &color )
    {
        QStringList cols;
        detail::column_dot_creator accu( cols, m_edges );
        boost::mpl::for_each<typename T::columns, detail::wrap<boost::mpl::placeholders::_1> >( accu );

        m_nodes.push_back(
            T::tableName()
            % QLatin1Literal( "[label=<<TABLE BORDER=\"1\" CELLBORDER=\"0\" CELLSPACING=\"0\" PORT=\"1\" BGCOLOR=\"" )
            % color
            % QLatin1Literal( "\">" )
            % QLatin1Literal( "<TR><TD BORDER=\"1\" ALIGN=\"CENTER\"><B>" )
            % T::tableName()
            % QLatin1Literal( "</B></TD></TR>" ) % cols.join( QLatin1String( " " ) ) % QLatin1Literal( "</TABLE>>];\n" )
        );
    }

    template <typename T>
    void makeRelationEdge( boost::mpl::false_ ) {}
    template <typename T>
    void makeRelationEdge( boost::mpl::true_ )
    {
        m_edges.push_back( T::leftType::referenced_column::table::tableName() % QLatin1Literal( ":" ) % T::leftType::referenced_column::sqlName()
            % QLatin1Literal( " -> " )
            % T::rightType::referenced_column::table::tableName() % QLatin1Literal( ":" ) % T::rightType::referenced_column::sqlName()
            % QLatin1Literal( "[label=\"n:m\" dir=both arrowtail=normal];\n" )
        );
    }

    QStringList &m_nodes;
    QStringList &m_edges;
};

} // detail


/**
 * Returns a dot representation of the database schema
 * @tparam Tables A MPL sequence of tables.
 */
template <typename Tables>
QString schemaToDot()
{
    QStringList nodes;
    QStringList edges;
    detail::table_dot_creator accu( nodes, edges );
    boost::mpl::for_each<Tables, detail::wrap<boost::mpl::placeholders::_1> >( accu );
    return QLatin1Literal( "digraph \"Database Schema\" {\n"
                           "graph [rankdir=\"LR\" fontsize=\"10\"]\n"
                           "node [fontsize=\"10\" shape=\"plaintext\"]\n"
                           "edge [fontsize=\"10\"]\n" ) %
            nodes.join( QLatin1String( "\n" ) ) %
            edges.join( QLatin1String( "\n" ) ) %
            QLatin1Literal( "}\n" );
}

}

#endif
