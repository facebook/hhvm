/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/hdf.h"

#include <boost/algorithm/string/predicate.hpp>

#include "hphp/util/lock.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Helper class storing HDF raw pointer and reference counts on it.
 */
class HdfRaw {
public:
  static Mutex HdfMutex;

  HdfRaw() : m_hdf(nullptr), m_count(1) {
    // ClearSilver is not thread-safe when calling hdf_init(), so guarding it.
    Lock lock(HdfMutex);
    Hdf::CheckNeoError(hdf_init(&m_hdf));
    assert(m_hdf);
  }
  ~HdfRaw() {
    if (m_hdf) {
      hdf_destroy(&m_hdf);
    }
  }

  HDF *m_hdf;
  int m_count;

  void inc() { m_count++;}
  void dec() { assert(m_count > 0); if (--m_count == 0) { delete this;}}
};

Mutex HdfRaw::HdfMutex;

///////////////////////////////////////////////////////////////////////////////
// constructors

Hdf::Hdf() : m_hdf(nullptr), m_dump(nullptr) {
  m_rawp = new HdfRaw();
}

Hdf::Hdf(const char *filename) : m_hdf(nullptr), m_dump(nullptr) {
  m_rawp = new HdfRaw();
  append(filename);
}

Hdf::Hdf(const std::string &filename) : m_hdf(nullptr), m_dump(nullptr) {
  m_rawp = new HdfRaw();
  append(filename.c_str());
}

Hdf::Hdf(const Hdf *hdf, const char *name) : m_hdf(nullptr), m_dump(nullptr) {
  assert(hdf);
  assert(name && *name);
  m_rawp = hdf->m_rawp;
  if (m_rawp) {
    m_rawp->inc();
    m_path = hdf->getFullPath();
    m_name = name;
  } else {
    assert(hdf->m_hdf);
    hdf_get_node(hdf->m_hdf, (char*)name, &m_hdf);
  }
}

Hdf::Hdf(const Hdf &hdf)
  : m_hdf(hdf.m_hdf), m_rawp(hdf.m_rawp), m_path(hdf.m_path),
    m_name(hdf.m_name), m_dump(nullptr) {
  if (m_rawp) {
    m_rawp->inc();
  }
}

Hdf::Hdf(HDF *hdf)
  : m_hdf(hdf), m_rawp(nullptr), m_dump(nullptr) {
}

Hdf::~Hdf() {
  if (m_rawp) {
    m_rawp->dec();
  }
  if (m_dump) {
    free(m_dump);
  }
}

void Hdf::assign(const Hdf &hdf) {
  m_hdf = hdf.m_hdf;
  if (m_rawp) {
    m_rawp->dec();
  }
  m_rawp = hdf.m_rawp;
  if (m_rawp) {
    m_rawp->inc();
  }
  m_path = hdf.m_path;
  m_name = hdf.m_name;
  if (m_dump) {
    free(m_dump);
    m_dump = nullptr;
  }
}

void Hdf::copy(const Hdf &hdf) {
  CheckNeoError(hdf_copy(getRaw(), nullptr, hdf.getRaw()));
}

void Hdf::open(const char *filename) {
  close();
  append(filename);
}

void Hdf::append(const char *filename) {
  assert(filename && *filename);
  if (!(boost::contains(filename, ".hdf")
    || boost::ends_with(filename, ".hphp"))) {
    return;
  }
  CheckNeoError(hdf_read_file(getRaw(), (char*)filename));
}

void Hdf::close() {
  m_hdf = nullptr;
  if (m_rawp) {
    m_rawp->dec();
    m_rawp = new HdfRaw();
  }
  m_path.clear();
  m_name.clear();
  if (m_dump) {
    free(m_dump);
    m_dump = nullptr;
  }
}

static bool match(const std::string &name, const std::string &pattern) {
  assert(!name.empty() && !pattern.empty());

  unsigned int len = pattern.size();
  char first = pattern[0];
  char last = pattern[len - 1];
  if (first == '*') {
    if (last == '*') {
      return name.find(pattern.substr(1, len - 2)) != std::string::npos;
    }
    return name.size() >= len - 1 &&
      name.substr(name.size() - len + 1) == pattern.substr(1);
  }
  if (last == '*') {
    return strncmp(name.c_str(), pattern.c_str(), len - 1) == 0;
  }
  return name == pattern;
}

bool Hdf::lintImpl(std::vector<std::string> &names,
                   const std::vector<std::string> &excludes, bool visited) {
  unsigned int size = names.size();

  bool childVisited = false;
  for (Hdf hdf = firstChild(false); hdf.exists(); hdf = hdf.next(false)) {
    if (hdf.lintImpl(names, excludes, visited)) {
      childVisited = true;
    }
  }
  bool meVisited = childVisited || hdf_is_visited(getRaw());

  std::string fullname = getFullPath();
  if (!fullname.empty()) {
    if (meVisited == visited) {
      bool excluded = false;
      for (unsigned int i = 0; i < excludes.size(); i++) {
        if (match(fullname, excludes[i])) {
          excluded = true;
          break;
        }
      }
      if (!excluded) {
        if (!visited) {
          names.resize(size); // so reports about my children are gone
        }
        names.push_back(fullname);
      }
    }
  }

  return meVisited;
}

void Hdf::lint(std::vector<std::string> &names,
               const char *excludePatternNode /* = "LintExcludePatterns" */,
               bool visited /* = false */) {
  std::vector<std::string> patterns;
  if (excludePatternNode && *excludePatternNode) {
    for (Hdf hdf = operator[](excludePatternNode).firstChild();
         hdf.exists(); hdf = hdf.next()) {
      std::string value = hdf.getString();
      if (!value.empty()) {
        patterns.push_back(value);
      }
    }
  }

  lintImpl(names, patterns, visited);
}

void Hdf::setVisited(bool visited /* = true */) {
  hdf_set_visited(getRaw(), visited ? 1 : 0);
  for (Hdf hdf = firstChild(false); hdf.exists(); hdf = hdf.next(false)) {
    hdf.setVisited(visited);
  }
}

///////////////////////////////////////////////////////////////////////////////
// gets

const char *Hdf::get(const char *defValue /* = NULL */) const {
  HDF *hdf = getRaw();
  const char *v = hdf_obj_value(hdf);
  hdf_set_visited(hdf, 1);
  return v ? v : defValue;
}

std::string Hdf::getString(const std::string &defValue /* = "" */) const {
  const char *v = get();
  if (v == nullptr) return defValue;
  return v;
}

bool Hdf::getBool(bool defValue /* = false */) const {
  const char *v = get();
  if (v == nullptr) return defValue;

  return *v && strcmp(v, "0") &&
    strcasecmp(v, "false") && strcasecmp(v, "no") && strcasecmp(v, "off");
}

int64_t Hdf::getInt(int64_t defValue, const char *type, int64_t maxValue) const {
  const char *v = get();
  if (v == nullptr) return defValue;

  char *endptr = nullptr;
  int64_t n = strtoll(v, &endptr, 0);
  if ((!endptr && !*endptr) ||
      (maxValue && (n > maxValue || n < (- maxValue - 1)))) {
    throw HdfDataTypeException(this, type, v);
  }

  return n;
}

char Hdf::getByte(char defValue /* = 0 */) const {
  return getInt(defValue, "byte", 0x7FL);
}

int16_t Hdf::getInt16(int16_t defValue /* = 0 */) const {
  return getInt(defValue, "int16", 0x7FFFL);
}

int32_t Hdf::getInt32(int32_t defValue /* = 0 */) const {
  return getInt(defValue, "int32", 0x7FFFFFFFL);
}

int64_t Hdf::getInt64(int64_t defValue /* = 0 */) const {
  return getInt(defValue, "int64", 0);
}

uint64_t Hdf::getUInt(uint64_t defValue, const char *type, uint64_t mask) const {
  const char *v = get();
  if (v == nullptr) return defValue;

  char *endptr = nullptr;
  int64_t n = strtoull(v, &endptr, 0);
  if ((!endptr && !*endptr) || (mask && ((uint64_t)n & mask))) {
    throw HdfDataTypeException(this, type, v);
  }

  return n;
}

unsigned char Hdf::getUByte(unsigned char defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned byte", ~0xFFUL);
}

uint16_t Hdf::getUInt16(uint16_t defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned int16", ~0xFFFFUL);
}

uint32_t Hdf::getUInt32(uint32_t defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned int32", ~0xFFFFFFFFUL);
}

uint64_t Hdf::getUInt64(uint64_t defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned int64", 0);
}

double Hdf::getDouble(double defValue /* = 0 */) const {
  const char *v = get();
  if (v == nullptr) return defValue;

  char *endptr = nullptr;
  double n = strtod(v, &endptr);
  if (!endptr && !*endptr) {
    throw HdfDataTypeException(this, "double", v);
  }

  return n;
}

void Hdf::get(std::vector<std::string> &values) const {
  values.clear();
  for (Hdf hdf = firstChild(); hdf.exists(); hdf = hdf.next()) {
    values.push_back(hdf.getString(""));
  }
}

void Hdf::get(std::set<std::string> &values) const {
  values.clear();
  for (Hdf hdf = firstChild(); hdf.exists(); hdf = hdf.next()) {
    values.insert(hdf.getString(""));
  }
}

void Hdf::get(boost::container::flat_set<std::string> &values) const {
  values.clear();
  for (Hdf hdf = firstChild(); hdf.exists(); hdf = hdf.next()) {
    values.insert(hdf.getString(""));
  }
}

void Hdf::get(std::set<std::string, stdltistr> &values) const {
  values.clear();
  for (Hdf hdf = firstChild(); hdf.exists(); hdf = hdf.next()) {
    values.insert(hdf.getString(""));
  }
}

void Hdf::get(std::map<std::string, std::string> &values) const {
  values.clear();
  for (Hdf hdf = firstChild(); hdf.exists(); hdf = hdf.next()) {
    values[hdf.getName()] = hdf.getString("");
  }
}

void Hdf::get(hphp_string_imap<std::string> &values) const {
  values.clear();
  for (Hdf hdf = firstChild(); hdf.exists(); hdf = hdf.next()) {
    values[hdf.getName()] = hdf.getString("");
  }
}

int Hdf::compare(const char *v2) const {
  const char *v1 = get();
  if (v1 == nullptr && v2 == nullptr) return 0;
  if (v1 == nullptr) return -1;
  if (v2 == nullptr) return 1;
  return strcmp(v1, v2);
}

int Hdf::compare(const std::string &v2) const {
  std::string v1 = getString();
  return strcmp(v1.c_str(), v2.c_str());
}

int Hdf::compare(char v2) const {
  char v1 = getByte();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(unsigned char v2) const {
  unsigned char v1 = getUByte();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(int16_t v2) const {
  int16_t v1 = getInt16();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(uint16_t v2) const {
  uint16_t v1 = getUInt16();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(int32_t v2) const {
  int32_t v1 = getInt32();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(uint32_t v2) const {
  uint32_t v1 = getUInt32();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(int64_t v2) const {
  int64_t v1 = getInt64();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(uint64_t v2) const {
  uint64_t v1 = getUInt64();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(double v2) const {
  double v1 = getDouble();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

///////////////////////////////////////////////////////////////////////////////
// sets

Hdf &Hdf::operator=(const Hdf &hdf) {
  if (&hdf != this) {
    if (m_rawp != hdf.m_rawp) {
      if (m_rawp) {
        m_rawp->dec();
      }
      m_rawp = hdf.m_rawp;
      if (m_rawp) {
        m_rawp->inc();
      }
    }

    m_hdf = hdf.m_hdf;
    m_path = hdf.m_path;
    m_name = hdf.m_name;

    if (m_dump) {
      free(m_dump);
    }
    m_dump = nullptr;
  }
  return *this;
}

void Hdf::set(const char *value) {
  CheckNeoError(hdf_set_value(getRaw(), nullptr, (char*)value));
}

void Hdf::set(int64_t value) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%lld", (long long)value);
  set(buf);
}

void Hdf::set(uint64_t value) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%llu", (unsigned long long)value);
  set(buf);
}

void Hdf::set(double value) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%g", value);
  set(buf);
}

///////////////////////////////////////////////////////////////////////////////
// sub-nodes

std::string Hdf::getName(bool markVisited /* = true */) const {
  HDF *hdf = getRaw();
  char *name = hdf_obj_name(hdf);
  if (markVisited) hdf_set_visited(hdf, 1);
  return name ? name : "";
}

std::string Hdf::getFullPath() const {
  std::string fullpath;
  if (m_path.empty()) {
    fullpath = m_name;
  } else {
    fullpath = m_path;
    if (!m_name.empty()) {
      fullpath += ".";
      fullpath += m_name;
    }
  }
  return fullpath;
}

Hdf Hdf::parentImpl() const {
  Hdf hdf(*this);
  if (m_name.empty()) {
    if (m_path.empty()) {
      throw HdfInvalidOperation("calling parent() on topmost node");
    }
    size_t pos = m_path.rfind('.');
    if (pos == std::string::npos) {
      hdf.m_name = m_path;
      hdf.m_path.clear();
    } else {
      hdf.m_name = m_path.substr(pos + 1);
      hdf.m_path = m_path.substr(0, pos);
    }
  } else {
    hdf.m_name.clear();
  }
  return hdf;
}

const Hdf Hdf::parent() const {
  return parentImpl();
}

Hdf Hdf::parent() {
  return parentImpl();
}

const Hdf Hdf::operator[](int name) const {
  char buf[12];
  sprintf(buf, "%d", name);
  return operator[](buf);
}

const Hdf Hdf::operator[](const char *name) const {
  return Hdf(this, name);
}

const Hdf Hdf::operator[](const std::string &name) const {
  return operator[](name.c_str());
}

Hdf Hdf::operator[](int name) {
  char buf[12];
  sprintf(buf, "%d", name);
  return operator[](buf);
}

Hdf Hdf::operator[](const char *name) {
  return Hdf(this, name);
}

Hdf Hdf::operator[](const std::string &name) {
  return operator[](name.c_str());
}

bool Hdf::exists() const {
  if (m_rawp == nullptr) {
    return m_hdf != nullptr;
  }

  std::string fullpath = getFullPath();
  if (fullpath.empty()) {
    return true;
  }
  return hdf_get_obj(m_rawp->m_hdf, fullpath.c_str());
}

bool Hdf::exists(int name) const {
  char buf[12];
  sprintf(buf, "%d", name);
  return exists(buf);
}

bool Hdf::exists(const char *name) const {
  HDF *hdf = m_hdf;
  if (m_rawp) {
    std::string fullpath = getFullPath();
    hdf = m_rawp->m_hdf;
    if (!fullpath.empty()) {
      hdf = hdf_get_obj(hdf, fullpath.c_str());
    }
  }
  return hdf && hdf_get_obj(hdf, name);
}

bool Hdf::exists(const std::string &name) const {
  return exists(name.c_str());
}

void Hdf::remove(int name) const {
  char buf[12];
  sprintf(buf, "%d", name);
  remove(buf);
}

void Hdf::remove(const char *name) const {
  assert(name && *name);
  CheckNeoError(hdf_remove_tree(getRaw(), name));
}

void Hdf::remove(const std::string &name) const {
  remove(name.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// iterations

Hdf Hdf::firstChild(bool markVisited /* = true */) const {
  HDF *hdf = getRaw();
  if (markVisited) hdf_set_visited(hdf, 1);
  Hdf ret(hdf_obj_child(hdf));
  ret.m_path = getFullPath();
  ret.m_name = ret.getName(markVisited);
  return ret;
}

Hdf Hdf::next(bool markVisited /* = true */) const {
  HDF *hdf = getRaw();
  if (markVisited) hdf_set_visited(hdf, 1);
  Hdf ret(hdf_obj_next(hdf));
  ret.m_path = m_path;
  ret.m_name = ret.getName(markVisited);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// input/output

void Hdf::fromString(const char *input) {
  CheckNeoError(hdf_read_string(getRaw(), (char*)input));
}

const char *Hdf::toString() const {
  if (m_dump) {
    free(m_dump);
    m_dump = nullptr;
  }
  CheckNeoError(hdf_write_string(getRaw(), &m_dump));
  return m_dump;
}

void Hdf::write(const char *filename) const {
  CheckNeoError(hdf_write_file(getRaw(), filename));
}

///////////////////////////////////////////////////////////////////////////////
// helpers

HDF *Hdf::getRaw() const {
  if (m_hdf) return m_hdf;

  if (m_rawp == nullptr) {
    return nullptr;
  }

  HDF *ret = nullptr;
  std::string fullpath = getFullPath();
  if (fullpath.empty()) {
    ret = m_rawp->m_hdf;
  } else {
    hdf_get_node(m_rawp->m_hdf, (char*)fullpath.c_str(), &ret);
  }
  m_hdf = ret;
  return ret;
}

void Hdf::CheckNeoError(NEOERR *err) {
  if (err != STATUS_OK) {
    NEOSTRING str;
    string_init(&str);
    nerr_error_string(err, &str);
    throw HdfException("%s", str.buf);
  }
}

///////////////////////////////////////////////////////////////////////////////
// exceptions

HdfException::HdfException(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
}

///////////////////////////////////////////////////////////////////////////////
}
