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

#ifndef __CONFIG_HDF_H__
#define __CONFIG_HDF_H__

#include "base.h"
#include <string>
#include "exception.h"
#include "neo/neo_hdf.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A super-fast hierarchical data structure, wrapped around ClearSilver's HDF
 * data format: http://www.clearsilver.net/docs/man_hdf.hdf
 *
 * HDF is a serialization format that emphasizes cleanness and simplicity when
 * representing hierarchical data. It's designed to be fast parsing and
 * accessing. One example is,
 *
 *   Server {
 *     Name = MyTestServer
 *     IP.1 = 192.168.100.100
 *     IP.2 = 192.168.100.101
 *   }
 */
class HdfRaw; // reference counting HDF* raw pointer, implmented in .cpp file
class Hdf {
public:
  /**
   * Constructors.
   */
  Hdf();                                 // create an empty HDF tree
  Hdf(const char *filename);             // open the specified file
  Hdf(const std::string &filename);      // open the specified file
  Hdf(const Hdf *hdf, const char *name); // constructing a sub-node  (internal)
  Hdf(const Hdf &hdf);                   // make a copy by reference (internal)
  Hdf(HDF *hdf);                         // attaching a raw pointer  (internal)
  ~Hdf();

  /**
   * Close current and make a copy of the specified.
   */
  void assign(const Hdf &hdf);

  /**
   * Copy specified without closing current.
   */
  void copy(const Hdf &hdf);

  /**
   * Either close current file and open a new file, or append a file's content.
   */
  void open(const char *filename);
  void open(const std::string &filename) { open(filename.c_str());}
  void append(const char *filename);
  void append(const std::string &filename) { append(filename.c_str());}
  void close();

  /**
   * Read or dump this entire tree in HDF format.
   */
  void fromString(const char *input);
  const char *toString() const;
  void write(const char *filename) const;
  void write(const std::string &filename) const { write(filename.c_str());}

  /**
   * Get this node's value. When node is not present or node's value is not
   * parsable, return default value "defValue" instead.
   *
   * Boolean "false" is defined as one of these values and anything else is
   * "true" (except absent node will take default value):
   *   1. empty string
   *   2. 0 (note: string "00" or longer are not "false").
   *   3. string "false", "no" or "off" case-insensitively
   *
   * Numbers can also be specified in hex (0x prefix) or octal (0 prefix). For
   * any values that are not entirely parsable to be a number, it will return
   * default value instead.
   */
  bool getBool(bool defValue = false) const;
  const char *get(const char *defValue = NULL) const;
  std::string getString(const std::string &defValue = "") const;
  char   getByte  (char   defValue = 0) const;
  uchar  getUByte (uchar  defValue = 0) const;
  int16  getInt16 (int16  defValue = 0) const;
  uint16 getUInt16(uint16 defValue = 0) const;
  int32  getInt32 (int32  defValue = 0) const;
  uint32 getUInt32(uint32 defValue = 0) const;
  int64  getInt64 (int64  defValue = 0) const;
  uint64 getUInt64(uint64 defValue = 0) const;
  double getDouble(double defValue = 0) const;

  void get(std::vector<std::string> &values) const;
  void get(std::set<std::string> &values) const;
  void get(std::map<std::string, std::string> &values) const;

  operator const char *() const { return get();}
  operator std::string() const { return getString();}
  operator bool   () const { return getBool()  ;}
  operator char   () const { return getByte()  ;}
  operator uchar  () const { return getUByte() ;}
  operator int16  () const { return getInt16() ;}
  operator uint16 () const { return getUInt16();}
  operator int32  () const { return getInt32() ;}
  operator uint32 () const { return getUInt32();}
  operator int64  () const { return getInt64() ;}
  operator uint64 () const { return getUInt64();}
  operator double () const { return getDouble();}

  /**
   * Set this node's value.
   */
  void set(const char *value);
  void set(const std::string &value) { set(value.c_str());}
  void set(bool   value) { set(value ? "1" : "0");}
  void set(char   value) { set((int64)value);}
  void set(uchar  value) { set((uint64)value);}
  void set(int16  value) { set((int64)value);}
  void set(uint16 value) { set((uint64)value);}
  void set(int32  value) { set((int64)value);}
  void set(uint32 value) { set((uint64)value);}
  void set(int64  value);
  void set(uint64 value);
  void set(double value);

  Hdf &operator=(const char *value) { set(value); return *this;}
  Hdf &operator=(const std::string &value) { set(value); return *this;}
  Hdf &operator=(bool   value) { set(value); return *this;}
  Hdf &operator=(char   value) { set(value); return *this;}
  Hdf &operator=(uchar  value) { set(value); return *this;}
  Hdf &operator=(int16  value) { set(value); return *this;}
  Hdf &operator=(uint16 value) { set(value); return *this;}
  Hdf &operator=(int32  value) { set(value); return *this;}
  Hdf &operator=(uint32 value) { set(value); return *this;}
  Hdf &operator=(int64  value) { set(value); return *this;}
  Hdf &operator=(uint64 value) { set(value); return *this;}
  Hdf &operator=(double value) { set(value); return *this;}

  /**
   * Get this node's fully qualified path or just one-level node name.
   */
  std::string getFullPath() const;
  std::string getName() const;

  /**
   * Get this node's parent.
   */
  const Hdf parent() const;
  Hdf parent();

  /**
   * Get a sub-node.
   */
  const Hdf operator[](int name) const;
  const Hdf operator[](const char *name) const;
  const Hdf operator[](const std::string &name) const;
  Hdf operator[](int name);
  Hdf operator[](const char *name);
  Hdf operator[](const std::string &name);

  /**
   * Note that this is different than getting a boolean value. If "name" is
   * present, testing whether a subnode exists. Otherwise, testing myself is
   * present or not.
   */
  bool exists() const;
  bool exists(int name) const;
  bool exists(const char *name) const;
  bool exists(const std::string &name) const;

  /**
   * Note that this is NOT testing existence, but reading a boolean value.
   */
  bool operator!() const { return !getBool();}

  /**
   * Removes a sub-node from parent.
   */
  void remove(int name) const;
  void remove(const char *name) const;
  void remove(const std::string &name) const;

  /**
   * Iterations. For example,
   *
   *   for (Hdf hdf = parent.firstChild(); hdf.exists(); hdf = hdf.next()) {
   *   }
   *
   * Please use "hdf.exists()" for testing than casting it to boolean.
   */
  Hdf firstChild() const;
  Hdf next() const;

  /**
   * Comparisons
   */
  int compare(const char *v) const;
  int compare(const std::string &v) const;
  int compare(char   v) const;
  int compare(uchar  v) const;
  int compare(int16  v) const;
  int compare(uint16 v) const;
  int compare(int32  v) const;
  int compare(uint32 v) const;
  int compare(int64  v) const;
  int compare(uint64 v) const;
  int compare(double v) const;

  bool operator==(const char *v) const { return compare(v) == 0;}
  bool operator!=(const char *v) const { return compare(v) != 0;}
  bool operator>=(const char *v) const { return compare(v) >= 0;}
  bool operator<=(const char *v) const { return compare(v) <= 0;}
  bool operator> (const char *v) const { return compare(v) >  0;}
  bool operator< (const char *v) const { return compare(v) <  0;}

  bool operator==(const std::string &v) const { return compare(v) == 0;}
  bool operator!=(const std::string &v) const { return compare(v) != 0;}
  bool operator>=(const std::string &v) const { return compare(v) >= 0;}
  bool operator<=(const std::string &v) const { return compare(v) <= 0;}
  bool operator> (const std::string &v) const { return compare(v) >  0;}
  bool operator< (const std::string &v) const { return compare(v) <  0;}

  bool operator==(char v) const { return compare(v) == 0;}
  bool operator!=(char v) const { return compare(v) != 0;}
  bool operator>=(char v) const { return compare(v) >= 0;}
  bool operator<=(char v) const { return compare(v) <= 0;}
  bool operator> (char v) const { return compare(v) >  0;}
  bool operator< (char v) const { return compare(v) <  0;}

  bool operator==(uchar v) const { return compare(v) == 0;}
  bool operator!=(uchar v) const { return compare(v) != 0;}
  bool operator>=(uchar v) const { return compare(v) >= 0;}
  bool operator<=(uchar v) const { return compare(v) <= 0;}
  bool operator> (uchar v) const { return compare(v) >  0;}
  bool operator< (uchar v) const { return compare(v) <  0;}

  bool operator==(int16 v) const { return compare(v) == 0;}
  bool operator!=(int16 v) const { return compare(v) != 0;}
  bool operator>=(int16 v) const { return compare(v) >= 0;}
  bool operator<=(int16 v) const { return compare(v) <= 0;}
  bool operator> (int16 v) const { return compare(v) >  0;}
  bool operator< (int16 v) const { return compare(v) <  0;}

  bool operator==(uint16 v) const { return compare(v) == 0;}
  bool operator!=(uint16 v) const { return compare(v) != 0;}
  bool operator>=(uint16 v) const { return compare(v) >= 0;}
  bool operator<=(uint16 v) const { return compare(v) <= 0;}
  bool operator> (uint16 v) const { return compare(v) >  0;}
  bool operator< (uint16 v) const { return compare(v) <  0;}

  bool operator==(int32 v) const { return compare(v) == 0;}
  bool operator!=(int32 v) const { return compare(v) != 0;}
  bool operator>=(int32 v) const { return compare(v) >= 0;}
  bool operator<=(int32 v) const { return compare(v) <= 0;}
  bool operator> (int32 v) const { return compare(v) >  0;}
  bool operator< (int32 v) const { return compare(v) <  0;}

  bool operator==(uint32 v) const { return compare(v) == 0;}
  bool operator!=(uint32 v) const { return compare(v) != 0;}
  bool operator>=(uint32 v) const { return compare(v) >= 0;}
  bool operator<=(uint32 v) const { return compare(v) <= 0;}
  bool operator> (uint32 v) const { return compare(v) >  0;}
  bool operator< (uint32 v) const { return compare(v) <  0;}

  bool operator==(int64 v) const { return compare(v) == 0;}
  bool operator!=(int64 v) const { return compare(v) != 0;}
  bool operator>=(int64 v) const { return compare(v) >= 0;}
  bool operator<=(int64 v) const { return compare(v) <= 0;}
  bool operator> (int64 v) const { return compare(v) >  0;}
  bool operator< (int64 v) const { return compare(v) <  0;}

  bool operator==(uint64 v) const { return compare(v) == 0;}
  bool operator!=(uint64 v) const { return compare(v) != 0;}
  bool operator>=(uint64 v) const { return compare(v) >= 0;}
  bool operator<=(uint64 v) const { return compare(v) <= 0;}
  bool operator> (uint64 v) const { return compare(v) >  0;}
  bool operator< (uint64 v) const { return compare(v) <  0;}

  bool operator==(double v) const { return compare(v) == 0;}
  bool operator!=(double v) const { return compare(v) != 0;}
  bool operator>=(double v) const { return compare(v) >= 0;}
  bool operator<=(double v) const { return compare(v) <= 0;}
  bool operator> (double v) const { return compare(v) >  0;}
  bool operator< (double v) const { return compare(v) <  0;}

  /**
   * Throw if there is an error from ClearSilver library.
   */
  static void CheckNeoError(NEOERR *err);

private:
  mutable HDF  *m_hdf ; // cached HDF pointer
  HdfRaw       *m_rawp; // raw pointer
  std::string   m_path; // parent path
  std::string   m_name; // my name
  mutable char *m_dump; // entire tree dump in HDF format

  /**
   * There are only two different "modes" of an Hdf object: hdf_ being null or
   * non-null. First case is when we proactively constructed an Hdf object by
   * either opening a file or starting from scratch by calling Hdf(). Second
   * case is when we attach a raw HDF*, almost exclusively used by iterations.
   */
  HDF *getRaw() const;

  /**
   * Parse value as a signed integer and check to make sure it's within
   * [-maxValue-1, maxValue]. If not, throw an HdfDataTypeException
   * with specified type string. If node is absent, return default value.
   */
  int64 getInt(int64 defValue, const char *type, int64 maxValue) const;

  /**
   * Parse value as a unsigned integer and check against mask to make sure
   * it's in the specified range. If not, throw an HdfDataTypeException
   * with specified type string. If node is absent, return default value.
   */
  uint64 getUInt(uint64 defValue, const char *type, uint64 mask) const;

  /**
   * Implementation of parent() calls.
   */
  Hdf parentImpl() const;
};

/**
 * Base class of all exceptions Hdf class might throw.
 */
class HdfException : public Exception {
public:
  HdfException(const char *fmt, ...) {
    va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
  }
};

/**
 * Trying to get a node's value, but it's not in the specified type.
 */
class HdfDataTypeException : public HdfException {
public:
  HdfDataTypeException(const Hdf *hdf, const char *type, const char *value)
    : HdfException("HDF node [%s]'s value \"%s\" is not %s",
                   hdf->getFullPath().c_str(), value, type) {
  }
};

/**
 * A node's value is not expected.
 */
class HdfDataValueException : public HdfException {
public:
  HdfDataValueException(const Hdf *hdf, const char *expected = "")
    : HdfException("HDF node [%s]'s value \"%s\" is not expected %s",
                   hdf->getFullPath().c_str(), hdf->get(""), expected) {
  }
};

/**
 * Calling a function in wrong context.
 */
class HdfInvalidOperation : public HdfException {
public:
  HdfInvalidOperation(const char *operation)
    : HdfException("Invalid operation: %s", operation) {
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __CONFIG_HDF_H__
