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

#include <string>
#include <vector>

namespace HPHP {

/*
 * Compare the type constraint and type structure resolutions present in the
 * repo hhbc_file to those derived from the source repository, src_repo. The
 * source repository must have a valid Facts autoloader.
 *
 * If paths is non-empty on the listed file-paths will be checked.
 *
 * On success exits zero, on failure exits HPHP_EXIT_FAILURE and reports errors
 * to STDERR.
 */
void compare_resolved_types(const std::string& hhbc_file,
                            const std::string& src_repo,
                            const std::vector<std::string>& paths);

}
