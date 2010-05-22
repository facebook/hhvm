/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/server/static_content_cache.h>
#include <runtime/base/runtime_option.h>
#include <util/timer.h>
#include <util/logger.h>
#include <util/process.h>
#include <util/util.h>
#include <util/compression.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

StaticContentCache StaticContentCache::TheCache;
FileCachePtr StaticContentCache::TheFileCache;

StaticContentCache::StaticContentCache() : m_totalSize(0) {
}

void StaticContentCache::load() {
  Timer timer(Timer::WallTime, "loading static content");

  if (!RuntimeOption::FileCache.empty()) {
    TheFileCache = FileCachePtr(new FileCache());
    TheFileCache->load(RuntimeOption::FileCache.c_str());
    Logger::Info("loaded file cache from %s",
                 RuntimeOption::FileCache.c_str());
    return;
  }

  int rootSize = RuntimeOption::SourceRoot.size();
  if (rootSize == 0) return;

  // get a list of all files, one for each extension
  Logger::Info("searching all files under source root...");
  int count = 0;
  map<string, vector<string> > ext2files;
  {
    const char *argv[] = {"", (char*)RuntimeOption::SourceRoot.c_str(),
                          "-type", "f", NULL};
    string files;
    vector<string> out;
    Process::Exec("find", argv, NULL, files);
    Util::split('\n', files.c_str(), out, true);
    for (unsigned int i = 0; i < out.size(); i++) {
      const string &name = out[i];
      size_t pos = name.rfind('.');
      if (pos != string::npos) {
        ext2files[name.substr(pos+1)].push_back(name);
        ++count;
      }
    }
  }

  Logger::Info("analyzing %d files under source root...", count);
  for (map<string, string>::const_iterator iter =
         RuntimeOption::StaticFileExtensions.begin();
       iter != RuntimeOption::StaticFileExtensions.end(); ++iter) {
    if (ext2files.find(iter->first) == ext2files.end()) {
      continue;
    }
    const vector<string> &out = ext2files[iter->first];

    int total = 0;
    for (unsigned int i = 0; i < out.size(); i++) {
      ResourceFilePtr f(new ResourceFile());

      StringBufferPtr sb(new StringBuffer(out[i].c_str()));
      if (sb->valid() && sb->size() > 0) {
        string url = out[i].substr(rootSize + 1);
        f->file = sb;
        m_files[url] = f;

        // prepare gzipped content, skipping image and swf files
        if (iter->second.find("image/") != 0 && iter->first != "swf") {
          int len = sb->size();
          char *data = gzencode(sb->data(), len, 9, CODING_GZIP);
          if (data) {
            if (len < sb->size()) {
              f->compressed = StringBufferPtr(new StringBuffer(data, len));
            } else {
              free(data);
            }
          }
        }

        total += sb->size();
      }
    }
    Logger::Info("..loaded %d bytes of %s files", total, iter->first.c_str());
    m_totalSize += total;
  }
  Logger::Info("loaded %d bytes of static content in total", m_totalSize);
}

bool StaticContentCache::find(const std::string &name, const char *&data,
                              int &len, bool &compressed) const {
  if (TheFileCache) {
    return data = TheFileCache->read(name.c_str(), len, compressed);
  }

  StringToResourceFilePtrMap::const_iterator iter = m_files.find(name);
  if (iter != m_files.end()) {
    if (compressed && iter->second->compressed) {
      data = iter->second->compressed->data();
      len = iter->second->compressed->size();
    } else {
      compressed = false;
      data = iter->second->file->data();
      len = iter->second->file->size();
    }
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
