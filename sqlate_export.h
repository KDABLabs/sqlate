#ifndef SQLATEEXPORT_H
#define SQLATEEXPORT_H

#include <qglobal.h>

# ifdef SQLATE_STATICLIB
#  undef SQLATE_SHAREDLIB
#  define SQLATE_EXPORT
# else
#  ifndef SQLATE_EXPORT
#   ifdef SQLATE_BUILD_SQLATE_LIB
#    define SQLATE_EXPORT Q_DECL_EXPORT
#   else
#    define SQLATE_EXPORT Q_DECL_IMPORT
#   endif
#  endif
# endif

#endif

