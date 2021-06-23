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

#ifndef incl_HPHP_ANNOTATION_CACHE_H_
#define incl_HPHP_ANNOTATION_CACHE_H_

#include "hphp/util/optional.h"

#include <unordered_map>
#include <string>

#include <folly/io/IOBuf.h>
#include <folly/Range.h>

namespace HPHP {

/*
 * This class is meant to be a utility for interacting with the annotations
 * produced by the printir utility, especially when the annotations are saved to
 * a separate file instead of being stored inline.
 */
struct AnnotationCache {
  struct FileInfo {
    const std::string fileName;
    const uint64_t offset; // offset into *uncompressed* data file
    const uint64_t length; // number of bytes of *uncompressed* data file
  };

  explicit AnnotationCache(const std::string& dumpDir) : m_dumpDir(dumpDir) {}

  /*
   * Given an annotation string, if the annotation is a redirect to a file,
   * use the file cache to return the string represented. Updates the file cache
   * with the current file if it is not already present. If the string is not a
   * redirect, return the string itself.
   */

  folly::StringPiece getAnnotation(const std::string& annotation);

  /*
   * Given an annotation string, if the annotation is a redirect to a file,
   * parse the redirect for the information required in FileInfo. Returns an
   * empty Optional if either the annotation string is not a redirect or if it
   * is improperly formatted.
   */
  Optional<FileInfo> getFileInfo(const std::string& annotation);

  /*
   * Given FileInfo, attempt to read the (decompressed) file in
   * m_dumpDir/fileName, and return the data stored in that file at the
   * position/length specified. Returns an empty Optional if file reading fails.
   *
   * If the file is successfully read, the entire file is cached in memory for
   * retrieval.
   *
   * If file reading fails, the failure is also cached so that we don't retry
   * reading the same broken file.
   */
  Optional<folly::StringPiece> getValue(const FileInfo&);

private:
  const uint8_t* getIOBuf(const std::string& fileName);

  const std::string m_dumpDir;
  std::unordered_map<std::string,
                     std::unique_ptr<const folly::IOBuf>> m_fileCache;
};

} // namespace HPHP

#endif
