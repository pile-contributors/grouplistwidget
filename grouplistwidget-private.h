/**
 * @file grouplistwidget-private.h
 * @brief Declarations for GroupListWidget class
 * @author Nicu Tofan <nicu.tofan@gmail.com>
 * @copyright Copyright 2014 piles contributors. All rights reserved.
 * This file is released under the
 * [MIT License](http://opensource.org/licenses/mit-license.html)
 */

#ifndef GUARD_GROUPLISTWIDGET_PRIVATE_H_INCLUDE
#define GUARD_GROUPLISTWIDGET_PRIVATE_H_INCLUDE

#include <grouplistwidget/grouplistwidget-config.h>

#ifndef DEBUG_OFF
#   define DEBUG_OFF 0
#endif // DEBUG_OFF

#ifndef DEBUG_ON
#   define DEBUG_ON 1
#endif // DEBUG_ON


#if DEBUG_ON
#    define GROUPLISTWIDGET_DEBUGM printf
#else
#    define GROUPLISTWIDGET_DEBUGM black_hole
#endif

#if DEBUG_OFF
#    define GROUPLISTWIDGET_TRACE_ENTRY printf("GROUPLISTWIDGET ENTRY %s in %s[%d]\n", __func__, __FILE__, __LINE__)
#else
#    define GROUPLISTWIDGET_TRACE_ENTRY
#endif

#if DEBUG_OFF
#    define GROUPLISTWIDGET_TRACE_EXIT printf("GROUPLISTWIDGET EXIT %s in %s[%d]\n", __func__, __FILE__, __LINE__)
#else
#    define GROUPLISTWIDGET_TRACE_EXIT
#endif


static inline void black_hole (...)
{}


#endif // GUARD_GROUPLISTWIDGET_PRIVATE_H_INCLUDE
