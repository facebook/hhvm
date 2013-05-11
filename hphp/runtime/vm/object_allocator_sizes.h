/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_OBJECT_ALLOCATOR_SIZES_H_
#define incl_HPHP_OBJECT_ALLOCATOR_SIZES_H_

namespace HPHP {
namespace VM {

// SmartAllocator (or, to be more specific, ObjectAllocator) is templatized by
// the size of the chunks it allocates. This works fine when we're only
// allocating objects of fixed sizes, known at compile time. However, the VM
// class "Instance" is of variable size (having room for a property vector at
// the end), and the range of sizes it will take on is unknowable at compile
// time.
//
// To work around this, we instantiate the ObjectAllocator template with a size
// from every possible size class. This way, any size class that Instance might
// end up falling into will have an ObjectAllocator.
int InitializeAllocators();

}}

#endif
