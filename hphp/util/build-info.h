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

#include "hphp/util/hash-map.h"
#include <folly/Range.h>

namespace HPHP {

/*
 * Version identifier for the hhbc repo schema.  Normally this is determined at
 * build-time, but it can be overridden at run-time via the
 * HHVM_RUNTIME_REPO_SCHEMA environment variable.
 */
folly::StringPiece repoSchemaId();

/*
 * Unique identifier for an hhvm binary, determined at build-time.  Normally
 * this is a formatted version control hash, but it can fall back to system time
 * in some cases.
 */
folly::StringPiece compilerId();

/*
 * Unix timestamp of the commit from which the compiler was built. Normally this
 * is the commit associated with compilerId but when no commit information was
 * available it may be the unix time of the build.
 */
int64_t compilerTimestamp();

/*
 * Unique identifier for this hhvm binary, determined at build-time. Unlike
 * compilerId(), this is computed based on the contents of the executable and
 * thus varies depending on the type of build. It cannot be overridden and
 * serves as an id for anything relying on an exact binary.
 */
folly::StringPiece buildId();

/*
 * Replace supported %{xxx} placeholders.  These include:
 *
 *  - %{schema} -> repo schema
 *  - %{uid} -> user id
 *  - %{euid} -> effective user id
 */
void replacePlaceholders(std::string&,
                         const hphp_fast_string_map<std::string>& replaces);
void replacePlaceholders(std::string&);

}

