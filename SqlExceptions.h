/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
*/
#ifndef SQLEXCEPTIONS_H
#define SQLEXCEPTIONS_H

#include "sqlate_export.h"

#include <exception>
#include <QSqlError>


class SQLATE_EXPORT SqlException : public std::exception {
public:    
    SqlException(const QSqlError& error) throw() : m_error( error ) {}
    virtual ~SqlException() throw(){};
    
    virtual const char* what() const throw() {
       return m_error.text().toLatin1().constData(); 
    }   
    
    QSqlError error() const {return m_error;}
    
private:
    QSqlError m_error;
};

#endif
