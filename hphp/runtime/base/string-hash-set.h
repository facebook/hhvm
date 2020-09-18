/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/base/string-functors.h"
#include "hphp/runtime/base/req-hash-set.h"

namespace HPHP {
namespace req {

// Set of case-insensitive Strings with iter/ref stability
using StringISet = req::hash_set<String,hphp_string_hash,hphp_string_isame>;

// Case-insensitive String keys, without iter/ref stability, but faster
using StringIFastSet = req::fast_set<String,hphp_string_hash,hphp_string_isame>;

// Case-sensitive String keys, no ref stability but faster
using StringFastSet = req::fast_set<String,hphp_string_hash,hphp_string_same>;

}}
