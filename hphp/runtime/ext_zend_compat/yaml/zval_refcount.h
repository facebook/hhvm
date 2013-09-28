/*
   +----------------------------------------------------------------------+
   | Zend Engine                                                          |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2007 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
   | Authors: Andi Gutmans <andi@zend.com>                                |
   |          Zeev Suraski <zeev@zend.com>                                |
   +----------------------------------------------------------------------+
*/

#ifndef Z_REFCOUNT_P

#define Z_REFCOUNT_PP(ppz)             ((*(ppz))->refcount)
#define Z_SET_REFCOUNT_PP(ppz, rc)     ((*(ppz))->refcount = rc)
#define Z_ADDREF_PP(ppz)             (++(*(ppz))->refcount)
#define Z_DELREF_PP(ppz)             (--(*(ppz))->refcount)
#define Z_ISREF_PP(ppz)                ((*(ppz))->is_ref)
#define Z_SET_ISREF_PP(ppz)            ((*(ppz))->is_ref = 1)
#define Z_UNSET_ISREF_PP(ppz)          ((*(ppz))->is_ref = 0)
#define Z_SET_ISREF_TO_PP(ppz, isref)  ((*(ppz))->is_ref = isref)

#define Z_REFCOUNT_P(pz)            ((pz)->refcount)
#define Z_SET_REFCOUNT_P(z, rc)     ((pz)->refcount = rc)
#define Z_ADDREF_P(pz)            (++(pz)->refcount)
#define Z_DELREF_P(pz)            (--(pz)->refcount)
#define Z_ISREF_P(pz)               ((pz)->is_ref)
#define Z_SET_ISREF_P(pz)           ((pz)->is_ref = 1)
#define Z_UNSET_ISREF_P(pz)         ((pz)->is_ref = 0)
#define Z_SET_ISREF_TO_P(z, isref)  ((pz)->is_ref = isref)

#define Z_REFCOUNT(z)             ((z).refcount)
#define Z_SET_REFCOUNT(z, rc)     ((z).refcount = rc)
#define Z_ADDREF(z)             (++(z).refcount)
#define Z_DELREF(z)             (--(z).refcount)
#define Z_ISREF(z)                ((z).is_ref)
#define Z_SET_ISREF(z)            ((z).is_ref = 1)
#define Z_UNSET_ISREF(z)          ((z).is_ref = 0)
#define Z_SET_ISREF_TO(z, isref)  ((z).is_ref = isref)

#endif              /* Z_REFCOUNT_P */
