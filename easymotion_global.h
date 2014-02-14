#ifndef EASYMOTION_GLOBAL_H
#define EASYMOTION_GLOBAL_H

#include <QtGlobal>

#if defined(EASYMOTION_LIBRARY)
#  define EASYMOTIONSHARED_EXPORT Q_DECL_EXPORT
#else
#  define EASYMOTIONSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // EASYMOTION_GLOBAL_H
