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

#include "hphp/tools/tc-print/annotation-cache.h"

#include <zlib.h>

#include <folly/Format.h>

#include "hphp/runtime/ext/zlib/ext_zlib.h"

#define BUFLEN 1000

using folly::IOBuf;
using std::string;

namespace HPHP {

using FileInfo = AnnotationCache::FileInfo;

Optional<FileInfo> AnnotationCache::getFileInfo(const string& annotation) {
  if (annotation.substr(0, 5) != "file:") return Optional<FileInfo>();

  string fileName = annotation.substr(5);

  // The actual file should be located in dumpDir.
  size_t pos = fileName.find_last_of('/');
  if (pos != std::string::npos) {
    fileName = fileName.substr(pos+1);
  }

  uint64_t offset = 0;
  uint64_t length = std::numeric_limits<decltype(length)>::max();
  pos = fileName.find_first_of(':');
  if (pos != string::npos) {
    auto filePos = fileName.substr(pos+1);
    fileName.resize(pos);
    if (sscanf(filePos.c_str(), "%ld:%ld", &offset, &length) != 2) {
      return Optional<FileInfo>();
    }
  }

  fileName = folly::sformat("{}/{}", m_dumpDir, fileName);

  return make_optional(FileInfo{fileName, offset, length});
}

const uint8_t* AnnotationCache::getIOBuf(const string& fileName) {
  auto const& cachedElem = m_fileCache.find(fileName);
  if (cachedElem != m_fileCache.end()) return cachedElem->second->data();

  FILE *file = fopen(fileName.c_str(), "r");
  if (!file) {
    m_fileCache.emplace(fileName, nullptr);
    return nullptr;
  }

  gzFile compressedFile = gzdopen(fileno(file), "r");
  if (!compressedFile) {
    m_fileCache.emplace(fileName, nullptr);
    fclose(file);
    return nullptr;
  }

  auto buf = std::make_unique<IOBuf>();
  while (1) {
    auto nextBuf = std::make_unique<IOBuf>(IOBuf::CreateOp::CREATE, BUFLEN);
    auto const readResult = gzread(compressedFile,
                                   nextBuf->writableData(),
                                   nextBuf->capacity());
    nextBuf->append(readResult);
    if (readResult == Z_NULL) break;
    buf->prev()->appendChain(std::move(nextBuf));
  }

  buf->coalesce();
  gzclose(compressedFile);
  auto const* data = buf->data();
  m_fileCache.emplace(fileName, std::move(buf));
  return data;
};

Optional<folly::StringPiece>
AnnotationCache::getValue(const FileInfo& fileInfo) {
  auto const ioBuf = getIOBuf(fileInfo.fileName);
  if (!ioBuf) return Optional<folly::StringPiece>();

  auto const cBuf = reinterpret_cast<const char*>(ioBuf);

  auto const start = cBuf + fileInfo.offset;
  auto const end = start + fileInfo.length;
  return folly::StringPiece(start, end);
}

folly::StringPiece
AnnotationCache::getAnnotation(const std::string& annotation) {
  auto const maybeFileInfo = getFileInfo(annotation);
  if (!maybeFileInfo) return annotation;

  auto const maybeValue = getValue(*maybeFileInfo);
  return maybeValue.value_or(annotation);
}

} // namespace HPHP
