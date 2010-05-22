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

#include "file_cache.h"
#include "exception.h"
#include "compression.h"
#include "util.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::string FileCache::SourceRoot;

///////////////////////////////////////////////////////////////////////////////
// helper

static bool read_bytes(FILE *f, char *buf, int len) {
  size_t nread = 0;
  while (len && (nread = fread(buf, 1, len, f)) != 0) {
    buf += nread;
    len -= nread;
  }
  return nread;
}

///////////////////////////////////////////////////////////////////////////////

FileCache::~FileCache() {
  for (FileMap::iterator iter = m_files.begin(); iter != m_files.end();
       ++iter) {
    Buffer &buffer = iter->second;
    if (buffer.data) {
      free(buffer.data);
    }
    if (buffer.cdata) {
      free(buffer.cdata);
    }
  }
}

void FileCache::writeDirectories(const char *name) {
  string sname = name;
  for (int i = 1; i < (int)sname.size(); i++) {
    if (sname[i] == '/') {
      string dir = sname.substr(0, i);
      if (!exists(dir.c_str())) {
        Buffer &buffer = m_files[dir];
        buffer.len = -2; // directory
        buffer.data = NULL;
        buffer.clen = -2;
        buffer.cdata = NULL;
      }
    }
  }
}

std::string FileCache::GetRelativePath(const char *path) {
  ASSERT(path);

  string relative = path;
  unsigned int len = SourceRoot.size();
  if (len > 0 && relative.size() > len &&
      strncmp(relative.data(), SourceRoot.c_str(), len) == 0) {
    relative = relative.substr(len);
  }
  if (!relative.empty() && relative[relative.length() - 1] == '/') {
    relative = relative.substr(0, relative.length() - 1);
  }
  return relative;
}

void FileCache::write(const char *name, bool addDirectories /* = true */) {
  ASSERT(name && *name);
  ASSERT(!exists(name));

  Buffer &buffer = m_files[name];
  buffer.len = -1; // PHP file
  buffer.data = NULL;
  buffer.clen = -1;
  buffer.cdata = NULL;

  if (addDirectories) {
    writeDirectories(name);
  }
}

void FileCache::write(const char *name, const char *fullpath) {
  ASSERT(name && *name);
  ASSERT(fullpath && *fullpath);
  ASSERT(!exists(name));

  struct stat sb;
  if (stat(fullpath, &sb) != 0) {
    throw Exception("Unable to stat %s: %s", fullpath,
                    Util::safe_strerror(errno).c_str());
  }
  int len = sb.st_size;
  Buffer &buffer = m_files[name];
  buffer.len = len; // static file
  buffer.data = NULL;
  buffer.clen = -1;
  buffer.cdata = NULL;

  if (len) {
    FILE *f = fopen(fullpath, "r");
    if (f == NULL) {
      throw Exception("Unable to open %s: %s", fullpath,
                      Util::safe_strerror(errno).c_str());
    }

    char *buf = buffer.data = (char *)malloc(len);
    if (!read_bytes(f, buf, len)) {
      throw Exception("Unable to read all bytes from %s", fullpath);
    }
    fclose(f);

    if (is_compressible_file(name)) {
      int new_len = buffer.len;
      char *compressed = gzencode(buffer.data, new_len, 9, CODING_GZIP);
      if (compressed) {
        if (new_len < buffer.len) {
          buffer.clen = new_len;
          buffer.cdata = compressed;
        } else {
          free(compressed);
        }
      }
    }
  }

  writeDirectories(name);
}

void FileCache::save(const char *filename) {
  ASSERT(filename && *filename);

  FILE *f = fopen(filename, "w");
  if (f == NULL) {
    throw Exception("Unable to open %s: %s", filename,
                    Util::safe_strerror(errno).c_str());
  }

  for (FileMap::const_iterator iter = m_files.begin(); iter != m_files.end();
       ++iter) {
    short name_len = iter->first.size();
    const char *name = iter->first.data();
    ASSERT(name_len);
    fwrite(&name_len, sizeof(short), 1, f);
    fwrite(name, name_len, 1, f);

    const Buffer &buffer = iter->second;
    char c = buffer.cdata ? 1 : 0;
    fwrite(&c, 1, 1, f);
    if (c) {
      ASSERT(buffer.clen > 0);
      fwrite(&buffer.clen, sizeof(int), 1, f);
      ASSERT(buffer.cdata);
      fwrite(buffer.cdata, buffer.clen, 1, f);
    } else {
      fwrite(&buffer.len, sizeof(int), 1, f);
      if (buffer.len > 0) {
        ASSERT(buffer.data);
        fwrite(buffer.data, buffer.len, 1, f);
      }
    }
  }

  fclose(f);
}

void FileCache::load(const char *filename) {
  ASSERT(filename && *filename);

  FILE *f = fopen(filename, "r");
  if (f == NULL) {
    throw Exception("Unable to open %s: %s", filename,
                    Util::safe_strerror(errno).c_str());
  }

  while (true) {
    short name_len;
    if (!read_bytes(f, (char*)&name_len, sizeof(short)) || name_len <= 0) {
      if (feof(f)) break;
      throw Exception("Bad file name length in archive %s", filename);
    }
    char *name = (char *)malloc(name_len + 1);
    if (!read_bytes(f, name, name_len)) {
      free(name);
      throw Exception("Bad file name in archive %s", filename);
    }
    name[name_len] = '\0';
    string file(name, name_len);
    free(name);
    if (exists(file.c_str())) {
      throw Exception("Same file %s appeared twice in %s", file.c_str(),
                      filename);
    }

    char c; int len;
    if (!read_bytes(f, (char*)&c, 1) ||
        !read_bytes(f, (char*)&len, sizeof(int))) {
      throw Exception("Bad data length in archive %s", filename);
    }

    Buffer &buffer = m_files[file];
    buffer.len = len;
    buffer.data = NULL;
    buffer.clen = -1;
    buffer.cdata = NULL;

    if (len > 0) {
      buffer.data = (char *)malloc(len + 1);
      if (!read_bytes(f, buffer.data, len)) {
        throw Exception("Bad data in archive %s", filename);
      }
      buffer.data[len] = '\0';
      if (c) {
        int new_len = buffer.len;
        char *uncompressed = gzdecode(buffer.data, new_len);
        if (uncompressed == NULL) {
          throw Exception("Bad compressed data in archive %s", filename);
        }

        buffer.clen = buffer.len;
        buffer.cdata = buffer.data;
        buffer.len = new_len;
        buffer.data = uncompressed;
      }
    }
  }
}

bool FileCache::fileExists(const char *name,
                           bool isRelative /* = true */) const {
  if (isRelative) {
    if (name && *name) {
      FileMap::const_iterator iter = m_files.find(name);
      if (iter != m_files.end() && iter->second.len >= -1) {
        return true;
      }
    }
    return false;
  }
  return fileExists(GetRelativePath(name).c_str());
}

bool FileCache::dirExists(const char *name,
                          bool isRelative /* = true */) const {
  if (isRelative) {
    if (name && *name) {
      FileMap::const_iterator iter = m_files.find(name);
      if (iter != m_files.end() && iter->second.len == -2) {
        return true;
      }
    }
    return false;
  }
  return dirExists(GetRelativePath(name).c_str());
}

bool FileCache::exists(const char *name,
                       bool isRelative /* = true */) const {
  if (isRelative) {
    if (name && *name) {
      return m_files.find(name) != m_files.end();
    }
    return false;
  }
  return exists(GetRelativePath(name).c_str());
}

char *FileCache::read(const char *name, int &len, bool &compressed) const {
  if (name && *name) {
    FileMap::const_iterator iter = m_files.find(name);
    if (iter != m_files.end()) {
      const Buffer &buf = iter->second;
      if (compressed && buf.cdata) {
        len = buf.clen;
        ASSERT(len > 0);
        return buf.cdata;
      }
      compressed = false;
      len = buf.len;
      if (len == 0) {
        ASSERT(buf.data == NULL);
        return "";
      }
      return buf.data;
    }
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
}
