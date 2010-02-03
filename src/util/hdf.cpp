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

#include "hdf.h"
#include "lock.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Helper class storing HDF raw pointer and reference counts on it.
 */
class HdfRaw {
public:
  static Mutex HdfMutex;

  HdfRaw() : m_hdf(NULL), m_count(1) {
    // ClearSilver is not thread-safe when calling hdf_init(), so guarding it.
    Lock lock(HdfMutex);
    Hdf::CheckNeoError(hdf_init(&m_hdf));
    ASSERT(m_hdf);
  }
  ~HdfRaw() {
    if (m_hdf) {
      hdf_destroy(&m_hdf);
    }
  }

  HDF *m_hdf;
  int m_count;

  void inc() { m_count++;}
  void dec() { if (--m_count == 0) { delete this;}}
};

Mutex HdfRaw::HdfMutex;

///////////////////////////////////////////////////////////////////////////////
// constructors

Hdf::Hdf() : m_hdf(NULL), m_dump(NULL) {
  m_rawp = new HdfRaw();
}

Hdf::Hdf(const char *filename) : m_hdf(NULL), m_dump(NULL) {
  m_rawp = new HdfRaw();
  append(filename);
}

Hdf::Hdf(const std::string &filename) : m_hdf(NULL), m_dump(NULL) {
  m_rawp = new HdfRaw();
  append(filename.c_str());
}

Hdf::Hdf(const Hdf *hdf, const char *name) : m_hdf(NULL), m_dump(NULL) {
  ASSERT(hdf);
  ASSERT(name && *name);
  m_rawp = hdf->m_rawp;
  if (m_rawp) {
    m_rawp->inc();
    m_path = hdf->getFullPath();
    m_name = name;
  } else {
    ASSERT(hdf->m_hdf);
    hdf_get_node(hdf->m_hdf, (char*)name, &m_hdf);
  }
}

Hdf::Hdf(const Hdf &hdf)
  : m_hdf(hdf.m_hdf), m_rawp(hdf.m_rawp), m_path(hdf.m_path),
    m_name(hdf.m_name), m_dump(NULL) {
  if (m_rawp) {
    m_rawp->inc();
  }
}

Hdf::Hdf(HDF *hdf)
  : m_hdf(hdf), m_rawp(NULL), m_dump(NULL) {
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
    m_dump = NULL;
  }
}

void Hdf::copy(const Hdf &hdf) {
  CheckNeoError(hdf_copy(getRaw(), NULL, hdf.getRaw()));
}

void Hdf::open(const char *filename) {
  close();
  append(filename);
}

void Hdf::append(const char *filename) {
  ASSERT(filename && *filename);
  CheckNeoError(hdf_read_file(getRaw(), (char*)filename));
}

void Hdf::close() {
  m_hdf = NULL;
  if (m_rawp) {
    m_rawp->dec();
    m_rawp = new HdfRaw();
  }
  m_path.clear();
  m_name.clear();
  if (m_dump) {
    free(m_dump);
    m_dump = NULL;
  }
}

///////////////////////////////////////////////////////////////////////////////
// gets

const char *Hdf::get(const char *defValue /* = NULL */) const {
  const char *v = hdf_obj_value(getRaw());
  return v ? v : defValue;
}

std::string Hdf::getString(const std::string &defValue /* = "" */) const {
  const char *v = get();
  if (v == NULL) return defValue;
  return v;
}

bool Hdf::getBool(bool defValue /* = false */) const {
  const char *v = get();
  if (v == NULL) return defValue;

  return *v && strcmp(v, "0") &&
    strcasecmp(v, "false") && strcasecmp(v, "no") && strcasecmp(v, "off");
}

int64 Hdf::getInt(int64 defValue, const char *type, int64 maxValue) const {
  const char *v = get();
  if (v == NULL) return defValue;

  char *endptr = NULL;
  int64 n = strtoll(v, &endptr, 0);
  if ((!endptr && !*endptr) ||
      (maxValue && (n > maxValue || n < (- maxValue - 1)))) {
    throw HdfDataTypeException(this, type, v);
  }

  return n;
}

char Hdf::getByte(char defValue /* = 0 */) const {
  return getInt(defValue, "byte", 0x7FL);
}

int16 Hdf::getInt16(int16 defValue /* = 0 */) const {
  return getInt(defValue, "int16", 0x7FFFL);
}

int32 Hdf::getInt32(int32 defValue /* = 0 */) const {
  return getInt(defValue, "int32", 0x7FFFFFFFL);
}

int64 Hdf::getInt64(int64 defValue /* = 0 */) const {
  return getInt(defValue, "int64", 0);
}

uint64 Hdf::getUInt(uint64 defValue, const char *type, uint64 mask) const {
  const char *v = get();
  if (v == NULL) return defValue;

  char *endptr = NULL;
  int64 n = strtoull(v, &endptr, 0);
  if ((!endptr && !*endptr) || (mask && ((uint64)n & mask))) {
    throw HdfDataTypeException(this, type, v);
  }

  return n;
}

uchar Hdf::getUByte(uchar defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned byte", ~0xFFUL);
}

uint16 Hdf::getUInt16(uint16 defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned int16", ~0xFFFFUL);
}

uint32 Hdf::getUInt32(uint32 defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned int32", ~0xFFFFFFFFUL);
}

uint64 Hdf::getUInt64(uint64 defValue /* = 0 */) const {
  return getUInt(defValue, "unsigned int64", 0);
}

double Hdf::getDouble(double defValue /* = 0 */) const {
  const char *v = get();
  if (v == NULL) return defValue;

  char *endptr = NULL;
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

void Hdf::get(std::map<std::string, std::string> &values) const {
  values.clear();
  for (Hdf hdf = firstChild(); hdf.exists(); hdf = hdf.next()) {
    values[hdf.getName()] = hdf.getString("");
  }
}

int Hdf::compare(const char *v2) const {
  const char *v1 = get();
  if (v1 == NULL && v2 == NULL) return 0;
  if (v1 == NULL) return -1;
  if (v2 == NULL) return 1;
  return strcmp(v1, v2);
}

int Hdf::compare(const std::string &v2) const {
  string v1 = getString();
  return strcmp(v1.c_str(), v2.c_str());
}

int Hdf::compare(char v2) const {
  char v1 = getByte();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(uchar v2) const {
  uchar v1 = getUByte();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(int16 v2) const {
  int16 v1 = getInt16();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(uint16 v2) const {
  uint16 v1 = getUInt16();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(int32 v2) const {
  int32 v1 = getInt32();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(uint32 v2) const {
  uint32 v1 = getUInt32();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(int64 v2) const {
  int64 v1 = getInt64();
  if (v1 == v2) return 0;
  return v1 > v2 ? 1 : -1;
}

int Hdf::compare(uint64 v2) const {
  uint64 v1 = getUInt64();
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

void Hdf::set(const char *value) {
  CheckNeoError(hdf_set_value(getRaw(), NULL, (char*)value));
}

void Hdf::set(int64 value) {
  char buf[24];
  snprintf(buf, sizeof(buf), "%lld", (long long)value);
  set(buf);
}

void Hdf::set(uint64 value) {
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

std::string Hdf::getName() const {
  char *name = hdf_obj_name(getRaw());
  return name ? name : "";
}

std::string Hdf::getFullPath() const {
  string fullpath;
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
    if (pos == string::npos) {
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
  if (m_rawp == NULL) {
    return m_hdf != NULL;
  }

  string fullpath = getFullPath();
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
  return Hdf(this, name).exists();
}

bool Hdf::exists(const std::string &name) const {
  return name.c_str();
}

void Hdf::remove(int name) const {
  char buf[12];
  sprintf(buf, "%d", name);
  remove(buf);
}

void Hdf::remove(const char *name) const {
  ASSERT(name && *name);
  CheckNeoError(hdf_remove_tree(getRaw(), name));
}

void Hdf::remove(const std::string &name) const {
  remove(name.c_str());
}

///////////////////////////////////////////////////////////////////////////////
// iterations

Hdf Hdf::firstChild() const {
  return hdf_obj_child(getRaw());
}

Hdf Hdf::next() const {
  return hdf_obj_next(getRaw());
}

///////////////////////////////////////////////////////////////////////////////
// input/output

void Hdf::fromString(const char *input) {
  CheckNeoError(hdf_read_string(getRaw(), (char*)input));
}

const char *Hdf::toString() const {
  if (m_dump) {
    free(m_dump);
    m_dump = NULL;
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

  if (m_rawp == NULL) {
    return NULL;
  }

  HDF *ret = NULL;
  string fullpath = getFullPath();
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
    STRING str;
    string_init(&str);
    nerr_error_string(err, &str);
    throw HdfException(str.buf);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
