/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CONFIG_HDF_H_
#define incl_HPHP_CONFIG_HDF_H_

#include "hphp/util/base.h"
#include <boost/container/flat_set.hpp>
#include <string>
#include "hphp/util/exception.h"
#include "hphp/util/case-insensitive.h"
#include "hphp/neo/neo_hdf.h"

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
  Hdf();                                          // create an empty HDF tree
  explicit Hdf(const char *filename);             // open the specified file
  explicit Hdf(const std::string &filename);      // open the specified file
  explicit Hdf(const Hdf *hdf, const char *name); // constructing a sub-node
           Hdf(const Hdf &hdf);                   // make a copy by reference
  explicit Hdf(HDF *hdf);                         // attaching a raw pointer
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
   * Get a list of node names that are visited or not visited. Great for lint
   * purpose, finding nodes that are invalid for configurations, for example.
   * Use exclusion to skip checking certain node names. For example,
   *
   *   LintExcludePatterns {
   *     * = FutureConfigName
   *     * = *endwith
   *     * = startwith*
   *     * = *containing*
   *   }
   *
   * The pattern is NOT a full regex, but only the simple 3 as above.
   *
   * When visited = true, return a list of nodes that are visited.
   */
  void lint(std::vector<std::string> &names,
            const char *excludePatternNode = "LintExcludePatterns",
            bool visited = false);
  void setVisited(bool visited = true);

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
  const char *get(const char *defValue = nullptr) const;
  std::string getString(const std::string &defValue = "") const;
  char   getByte  (char   defValue = 0) const;
  uchar  getUByte (uchar  defValue = 0) const;
  int16_t  getInt16 (int16_t  defValue = 0) const;
  uint16_t getUInt16(uint16_t defValue = 0) const;
  int32_t  getInt32 (int32_t  defValue = 0) const;
  uint32_t getUInt32(uint32_t defValue = 0) const;
  int64_t  getInt64 (int64_t  defValue = 0) const;
  uint64_t getUInt64(uint64_t defValue = 0) const;
  double getDouble(double defValue = 0) const;

  void get(std::vector<std::string> &values) const;
  void get(std::set<std::string> &values) const;
  void get(std::set<std::string, stdltistr> &values) const;
  void get(boost::container::flat_set<std::string> &values) const;
  void get(std::map<std::string, std::string> &values) const;
  void get(hphp_string_imap<std::string> &values) const;

  /**
   * Set this node's value.
   */
  void set(const char *value);
  void set(const std::string &value) { set(value.c_str());}
  void set(bool   value) { set(value ? "1" : "0");}
  void set(char   value) { set((int64_t)value);}
  void set(uchar  value) { set((uint64_t)value);}
  void set(int16_t  value) { set((int64_t)value);}
  void set(uint16_t value) { set((uint64_t)value);}
  void set(int32_t  value) { set((int64_t)value);}
  void set(uint32_t value) { set((uint64_t)value);}
  void set(int64_t  value);
  void set(uint64_t value);
  void set(double value);

  Hdf &operator=(const char *value) { set(value); return *this;}
  Hdf &operator=(const std::string &value) { set(value); return *this;}
  Hdf &operator=(bool   value) { set(value); return *this;}
  Hdf &operator=(char   value) { set(value); return *this;}
  Hdf &operator=(uchar  value) { set(value); return *this;}
  Hdf &operator=(int16_t  value) { set(value); return *this;}
  Hdf &operator=(uint16_t value) { set(value); return *this;}
  Hdf &operator=(int32_t  value) { set(value); return *this;}
  Hdf &operator=(uint32_t value) { set(value); return *this;}
  Hdf &operator=(int64_t  value) { set(value); return *this;}
  Hdf &operator=(uint64_t value) { set(value); return *this;}
  Hdf &operator=(double value) { set(value); return *this;}
  Hdf &operator=(const Hdf &hdf);

  /**
   * Get this node's fully qualified path or just one-level node name.
   */
  std::string getFullPath() const;
  std::string getName(bool markVisited = true) const;

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
  Hdf firstChild(bool markVisited = true) const;
  Hdf next(bool markVisited = true) const;

  /**
   * Comparisons
   */
  int compare(const char *v) const;
  int compare(const std::string &v) const;
  int compare(char   v) const;
  int compare(uchar  v) const;
  int compare(int16_t  v) const;
  int compare(uint16_t v) const;
  int compare(int32_t  v) const;
  int compare(uint32_t v) const;
  int compare(int64_t  v) const;
  int compare(uint64_t v) const;
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

  bool operator==(int16_t v) const { return compare(v) == 0;}
  bool operator!=(int16_t v) const { return compare(v) != 0;}
  bool operator>=(int16_t v) const { return compare(v) >= 0;}
  bool operator<=(int16_t v) const { return compare(v) <= 0;}
  bool operator> (int16_t v) const { return compare(v) >  0;}
  bool operator< (int16_t v) const { return compare(v) <  0;}

  bool operator==(uint16_t v) const { return compare(v) == 0;}
  bool operator!=(uint16_t v) const { return compare(v) != 0;}
  bool operator>=(uint16_t v) const { return compare(v) >= 0;}
  bool operator<=(uint16_t v) const { return compare(v) <= 0;}
  bool operator> (uint16_t v) const { return compare(v) >  0;}
  bool operator< (uint16_t v) const { return compare(v) <  0;}

  bool operator==(int32_t v) const { return compare(v) == 0;}
  bool operator!=(int32_t v) const { return compare(v) != 0;}
  bool operator>=(int32_t v) const { return compare(v) >= 0;}
  bool operator<=(int32_t v) const { return compare(v) <= 0;}
  bool operator> (int32_t v) const { return compare(v) >  0;}
  bool operator< (int32_t v) const { return compare(v) <  0;}

  bool operator==(uint32_t v) const { return compare(v) == 0;}
  bool operator!=(uint32_t v) const { return compare(v) != 0;}
  bool operator>=(uint32_t v) const { return compare(v) >= 0;}
  bool operator<=(uint32_t v) const { return compare(v) <= 0;}
  bool operator> (uint32_t v) const { return compare(v) >  0;}
  bool operator< (uint32_t v) const { return compare(v) <  0;}

  bool operator==(int64_t v) const { return compare(v) == 0;}
  bool operator!=(int64_t v) const { return compare(v) != 0;}
  bool operator>=(int64_t v) const { return compare(v) >= 0;}
  bool operator<=(int64_t v) const { return compare(v) <= 0;}
  bool operator> (int64_t v) const { return compare(v) >  0;}
  bool operator< (int64_t v) const { return compare(v) <  0;}

  bool operator==(uint64_t v) const { return compare(v) == 0;}
  bool operator!=(uint64_t v) const { return compare(v) != 0;}
  bool operator>=(uint64_t v) const { return compare(v) >= 0;}
  bool operator<=(uint64_t v) const { return compare(v) <= 0;}
  bool operator> (uint64_t v) const { return compare(v) >  0;}
  bool operator< (uint64_t v) const { return compare(v) <  0;}

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
  int64_t getInt(int64_t defValue, const char *type, int64_t maxValue) const;

  /**
   * Parse value as a unsigned integer and check against mask to make sure
   * it's in the specified range. If not, throw an HdfDataTypeException
   * with specified type string. If node is absent, return default value.
   */
  uint64_t getUInt(uint64_t defValue, const char *type, uint64_t mask) const;

  /**
   * Implementation of parent() calls.
   */
  Hdf parentImpl() const;

  bool lintImpl(std::vector<std::string> &names,
                const std::vector<std::string> &excludes, bool visited);
};

/**
 * Base class of all exceptions Hdf class might throw.
 */
class HdfException : public Exception {
public:
  HdfException(const char *fmt, ...) ATTRIBUTE_PRINTF(2,3);
  EXCEPTION_COMMON_IMPL(HdfException);
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
  EXCEPTION_COMMON_IMPL(HdfDataTypeException);
};

/**
 * A node's value is not expected.
 */
class HdfDataValueException : public HdfException {
public:
  explicit HdfDataValueException(const Hdf *hdf, const char *expected = "")
    : HdfException("HDF node [%s]'s value \"%s\" is not expected %s",
                   hdf->getFullPath().c_str(), hdf->get(""), expected) {
  }
  EXCEPTION_COMMON_IMPL(HdfDataValueException);
};

/**
 * Calling a function in wrong context.
 */
class HdfInvalidOperation : public HdfException {
public:
  explicit HdfInvalidOperation(const char *operation)
    : HdfException("Invalid operation: %s", operation) {
  }
  EXCEPTION_COMMON_IMPL(HdfInvalidOperation);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CONFIG_HDF_H_
