/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/ext_fb.h>
#include <runtime/ext/ext_function.h>
#include <util/db_conn.h>
#include <netinet/in.h>
#include <runtime/base/externals.h>
#include <runtime/base/string_util.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/eval/runtime/code_coverage.h>
#include <runtime/base/runtime_option.h>

using namespace std;

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(fb);
///////////////////////////////////////////////////////////////////////////////

#define FB_UNSERIALIZE_NONSTRING_VALUE           0x0001
#define FB_UNSERIALIZE_UNEXPECTED_END            0x0002
#define FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE  0x0003
#define FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE 0x0004

const int64 k_FB_UNSERIALIZE_NONSTRING_VALUE = FB_UNSERIALIZE_NONSTRING_VALUE;
const int64 k_FB_UNSERIALIZE_UNEXPECTED_END = FB_UNSERIALIZE_UNEXPECTED_END;
const int64 k_FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE =
  FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
const int64 k_FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE =
  FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE;

///////////////////////////////////////////////////////////////////////////////

/* Linux and other systems don't currently support a ntohx or htonx
   set of functions for 64-bit values.  We've implemented our own here
   which is based off of GNU Net's implementation with some slight
   modifications (changed to macro's rather than functions). */
#if __BYTE_ORDER == __BIG_ENDIAN
#define ntohll(n) (n)
#define htonll(n) (n)
#else
#define ntohll(n) ( (((uint64_t)ntohl(n)) << 32) | ((uint64_t)ntohl(n >> 32) & 0x00000000ffffffff) )
#define htonll(n) ( (((uint64_t)htonl(n)) << 32) | ((uint64_t)htonl(n >> 32) & 0x00000000ffffffff) )
#endif

/* enum of thrift types */
enum TType {
  T_STOP    = 1,
  T_BYTE    = 2,
  T_U16     = 3,
  T_I16     = 4,
  T_U32     = 5,
  T_I32     = 6,
  T_U64     = 7,
  T_I64     = 8,
  T_STRING  = 9,
  T_STRUCT  = 10,
  T_MAP     = 11,
  T_SET     = 12,
  T_LIST    = 13,
  T_NULL    = 14,
  T_VARCHAR = 15,
  T_DOUBLE  = 16,
  T_BOOLEAN = 17,
};

/* Return the smallest size int that can store the value */
#define INT_SIZE(x) (((x) == ((int8_t)x))  ? 1 :    \
                     ((x) == ((int16_t)x)) ? 2 :    \
                     ((x) == ((int32_t)x)) ? 4 : 8)

/* Return the smallest (supported) unsigned length that can store the value */
#define LEN_SIZE(x) ((((unsigned)x) == ((uint8_t)x)) ? 1 : 4)

static int fb_serialized_size(CVarRef thing, int depth, int *bytes) {
  if (depth > 256) {
    return 1;
  }

  /* Get the size for an object, including one byte for the type */
  switch (thing.getType()) {
  case KindOfNull:      *bytes = 1; break;     /* type */
  case KindOfBoolean:   *bytes = 2; break;    /* type + sizeof(char) */
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:     *bytes = 1 + INT_SIZE(thing.toInt64()); break;
  case KindOfDouble:    *bytes = 9; break;     /* type + sizeof(double) */
  case LiteralString:
  case KindOfString:
    {
      int len = thing.toString().size();
      *bytes = 1 + LEN_SIZE(len) + len;
      break;
    }
  case KindOfArray:
    {
      int size = 2;
      Array arr = thing.toArray();
      for (ArrayIter iter(arr); iter; ++iter) {
        Variant key = iter.first();
        if (key.isNumeric()) {
          int64 index = key.toInt64();
          size += 1 + INT_SIZE(index);
        } else {
          int len = key.toString().size();
          size += 1 + LEN_SIZE(len) + len;
        }
        int additional_bytes = 0;
        if (fb_serialized_size(iter.second(), depth + 1,
                               &additional_bytes)) {
          return 1;
        }
        size += additional_bytes;
      }
      *bytes = size;
      break;
    }
  default:
    return 1;
  }
  return 0;
}

static void fb_serialize_long_into_buffer(int64 val, char *buff, int *pos) {
  switch (INT_SIZE(val)) {
  case 1:
    buff[(*pos)++] = T_BYTE;
    buff[(*pos)++] = (int8_t)val;
    break;
  case 2:
    buff[(*pos)++] = T_I16;
    *(int16_t *)(buff + (*pos)) = htons(val);
    (*pos) += 2;
    break;
  case 4:
    buff[(*pos)++] = T_I32;
    *(int32_t *)(buff + (*pos)) = htonl(val);
    (*pos) += 4;
    break;
  case 8:
    buff[(*pos)++] = T_I64;
    *(int64_t *)(buff + (*pos)) = htonll(val);
    (*pos) += 8;
    break;
  }
}

static void fb_serialize_string_into_buffer(CStrRef str, char *buf, int *pos) {
  int len = str.size();
  switch (LEN_SIZE(len)) {
  case 1:
    buf[(*pos)++] = T_VARCHAR;
    buf[(*pos)++] = (uint8_t)len;
    break;
  case 4:
    buf[(*pos)++] = T_STRING;
    *(uint32_t *)(buf + (*pos)) = htonl(len);
    (*pos) += 4;
    break;
  }

  /* memcpy the string into the buffer */
  memcpy(buf + (*pos), str.data(), len);
  (*pos) += len;
}

static bool fb_serialize_into_buffer(CVarRef thing, char *buff, int *pos) {
  switch (thing.getType()) {
  case KindOfNull:
    buff[(*pos)++] = T_NULL;
    break;
  case KindOfBoolean:
    buff[(*pos)++] = T_BOOLEAN;
    buff[(*pos)++] = (int8_t)thing.toInt64();
    break;
  case KindOfByte:
  case KindOfInt16:
  case KindOfInt32:
  case KindOfInt64:
    fb_serialize_long_into_buffer(thing.toInt64(), buff, pos);
    break;
  case KindOfDouble:
    buff[(*pos)++] = T_DOUBLE;
    *(double *)(buff + (*pos)) = thing.toDouble();
    (*pos) += 8;
    break;
  case LiteralString:
  case KindOfString:
    fb_serialize_string_into_buffer(thing.toString(), buff, pos);
    break;
  case KindOfArray:
    {
      buff[(*pos)++] = T_STRUCT;
      Array arr = thing.toArray();
      for (ArrayIter iter(arr); iter; ++iter) {
        Variant key = iter.first();
        if (key.isNumeric()) {
          int64 index = key.toInt64();
          fb_serialize_long_into_buffer(index, buff, pos);
        } else {
          fb_serialize_string_into_buffer(key.toString(), buff, pos);
        }

        if (!fb_serialize_into_buffer(iter.second(), buff, pos)) {
          return false;
        }
      }

      /* Write the final stop marker */
      buff[(*pos)++] = T_STOP;
    }
    break;
  default:
    raise_warning("unserializable object unexpectedly passed through "
                  "fb_serialized_size");
    ASSERT(false);
  }
  return true;
}

/* Check if there are enough bytes left in the buffer */
#define CHECK_ENOUGH(bytes, pos, num) do {                  \
    if ((int)(bytes) > (int)((num) - (pos))) {              \
      return FB_UNSERIALIZE_UNEXPECTED_END;                 \
    }                                                       \
  } while (0)

int fb_unserialize_from_buffer(Variant &res, const char *buff,
                               int buff_len, int *pos) {

  /* Check we have at least 1 byte for the type */
  CHECK_ENOUGH(1, *pos, buff_len);

  int type;
  switch (type = buff[(*pos)++]) {
  case T_NULL:
    res = null;
    break;
  case T_BOOLEAN:
    CHECK_ENOUGH(sizeof(int8_t), *pos, buff_len);
    res = (bool)(int8_t)buff[(*pos)++];
    break;
  case T_BYTE:
    CHECK_ENOUGH(sizeof(int8_t), *pos, buff_len);
    res = (int8_t)buff[(*pos)++];
    break;
  case T_I16:
    {
      CHECK_ENOUGH(sizeof(int16_t), *pos, buff_len);
      int16_t ret = (int16_t)ntohs(*(int16_t *)(buff + (*pos)));
      (*pos) += 2;
      res = ret;
      break;
    }
  case T_I32:
    {
      CHECK_ENOUGH(sizeof(int32_t), *pos, buff_len);
      int32_t ret = (int32_t)ntohl(*(int32_t *)(buff + (*pos)));
      (*pos) += 4;
      res = ret;
      break;
    }
  case T_I64:
    {
      CHECK_ENOUGH(sizeof(int64_t), *pos, buff_len);
      int64_t ret = (int64_t)ntohll(*(int64_t *)(buff + (*pos)));
      (*pos) += 8;
      res = (int64)ret;
      break;
    }
  case T_DOUBLE:
    {
      CHECK_ENOUGH(sizeof(double), *pos, buff_len);
      double ret = *(double *)(buff + (*pos));
      (*pos) += 8;
      res = ret;
      break;
    }
  case T_VARCHAR:
    {
      CHECK_ENOUGH(sizeof(uint8_t), *pos, buff_len);
      int len = (uint8_t)buff[(*pos)++];

      CHECK_ENOUGH(len, *pos, buff_len);
      StringData* ret = NEW(StringData)(buff + (*pos), len, CopyString);
      (*pos) += len;
      res = ret;
      break;
    }
  case T_STRING:
    {
      CHECK_ENOUGH(sizeof(uint32_t), *pos, buff_len);
      int len = (uint32_t)ntohl(*(uint32_t *)(buff + (*pos)));
      (*pos) += 4;

      CHECK_ENOUGH(len, *pos, buff_len);
      StringData* ret = NEW(StringData)(buff + (*pos), len, CopyString);
      (*pos) += len;
      res = ret;
      break;
    }
  case T_STRUCT:
    {
      Array ret = Array::Create();
      /* Need at least 1 byte for type/stop */
      CHECK_ENOUGH(1, *pos, buff_len);
      while ((type = buff[(*pos)++]) != T_STOP) {
        String key;
        int64 index = 0;
        switch(type) {
        case T_BYTE:
          CHECK_ENOUGH(sizeof(int8_t), *pos, buff_len);
          index = (int8_t)buff[(*pos)++];
          break;
        case T_I16:
          {
            CHECK_ENOUGH(sizeof(int16_t), *pos, buff_len);
            index = (int16_t)ntohs(*(int16_t *)(buff + (*pos)));
            (*pos) += 2;
            break;
          }
        case T_I32:
          {
            CHECK_ENOUGH(sizeof(int32_t), *pos, buff_len);
            index = (int32_t)ntohl(*(int32_t *)(buff + (*pos)));
            (*pos) += 4;
            break;
          }
        case T_I64:
          {
            CHECK_ENOUGH(sizeof(int64_t), *pos, buff_len);
            index = (int64_t)ntohll(*(int64_t *)(buff + (*pos)));
            (*pos) += 8;
            break;
          }
        case T_VARCHAR:
          {
            CHECK_ENOUGH(sizeof(uint8_t), *pos, buff_len);
            int len = (uint8_t)buff[(*pos)++];

            CHECK_ENOUGH(len, *pos, buff_len);
            key.assign(buff + (*pos), len, CopyString);
            (*pos) += len;
            break;
          }
        case T_STRING:
          {
            CHECK_ENOUGH(sizeof(uint32_t), *pos, buff_len);
            int len = (uint32_t)ntohl(*(uint32_t *)(buff + (*pos)));
            (*pos) += 4;

            CHECK_ENOUGH(len, *pos, buff_len);
            key.assign(buff + (*pos), len, CopyString);
            (*pos) += len;
            break;
          }
        default:
          return FB_UNSERIALIZE_UNEXPECTED_ARRAY_KEY_TYPE;
        }

        Variant value;
        int retval;
        if ((retval = fb_unserialize_from_buffer(value, buff, buff_len, pos))) {
          return retval;
        }
        if (!key.isNull()) {
          ret.set(key, value);
        } else {
          ret.set(index, value);
        }
        /* Need at least 1 byte for type/stop (see start of loop) */
        CHECK_ENOUGH(1, *pos, buff_len);
      }
      res = ret;
    }
    break;
  default:
    return FB_UNSERIALIZE_UNRECOGNIZED_OBJECT_TYPE;
  }

  return 0;
}

Variant f_fb_thrift_serialize(CVarRef thing) {
  int len;
  if (fb_serialized_size(thing, 0, &len)) {
    return null;
  }
  char *buff = (char *)malloc(len + 1);
  int pos = 0;
  fb_serialize_into_buffer(thing, buff, &pos);

  ASSERT(pos == len);
  buff[len] = '\0';
  return String(buff, len, AttachString);
}

Variant f_fb_thrift_unserialize(CVarRef thing, Variant success,
                                Variant errcode /* = null_variant */) {
  int pos = 0;
  errcode = null;
  int errcd;
  Variant ret;
  success = false;
  if (thing.isString()) {
    String sthing = thing.toString();
    if ((errcd = fb_unserialize_from_buffer(ret, sthing.data(), sthing.size(),
                                            &pos))) {
      errcode = errcd;
    } else {
      success = true;
      return ret;
    }
  } else {
    errcode = FB_UNSERIALIZE_NONSTRING_VALUE;
  }
  return false;
}

Variant f_fb_serialize(CVarRef thing) {
  return f_fb_thrift_serialize(thing);
}

Variant f_fb_unserialize(CVarRef thing, Variant success,
                         Variant errcode /* = null_variant */) {
  return f_fb_thrift_unserialize(thing, ref(success), ref(errcode));
}

///////////////////////////////////////////////////////////////////////////////

static void output_dataset(Array &ret, int affected, DBDataSet &ds,
                           const map<int, string> &errors) {
  ret.set("affected", affected);

  Array rows;
  MYSQL_FIELD *fields = ds.getFields();
  for (ds.moveFirst(); ds.getRow(); ds.moveNext()) {
    Array row;
    for (int i = 0; i < ds.getColCount(); i++) {
      const char *field = ds.getField(i);
      int len = ds.getFieldLength(i);
      if (field == NULL) field = "";
      row.set(String(fields[i].name, CopyString),
              String(field, len, CopyString));
    }
    rows.append(row);
  }
  ret.set("result", rows);

  if (!errors.empty()) {
    Array error;
    for (map<int, string>::const_iterator iter = errors.begin();
         iter != errors.end(); ++iter) {
      error.set(iter->first, String(iter->second));
    }
    ret.set("error", error);
  }
}

void f_fb_load_local_databases(CArrRef servers) {
  DBConn::clearLocalDatabases();
  for (ArrayIter iter(servers); iter; ++iter) {
    int dbId = iter.first().toInt32();
    Array data = iter.second().toArray();
    if (!data.empty()) {
      DBConn::addLocalDB(dbId, data["ip"].toString().data(),
                         data["db"].toString().data(),
                         data["port"].toInt32(),
                         data["username"].toString().data(),
                         data["password"].toString().data());
    }
  }
}

Array f_fb_parallel_query(CArrRef sql_map, int max_thread /* = 50 */,
                          bool combine_result /* = true */,
                          bool retry_query_on_fail /* = true */,
                          int connect_timeout /* = -1 */,
                          int read_timeout /* = -1 */,
                          bool timeout_in_ms /* = false */) {
  if (!timeout_in_ms) {
    if (connect_timeout > 0) connect_timeout *= 1000;
    if (read_timeout > 0) read_timeout *= 1000;
  }

  ServerQueryVec queries;
  for (ArrayIter iter(sql_map); iter; ++iter) {
    Array data = iter.second().toArray();
    if (!data.empty()) {
      ServerDataPtr server
        (new ServerData(data["ip"].toString().data(),
                        data["db"].toString().data(),
                        data["port"].toInt32(),
                        data["username"].toString().data(),
                        data["password"].toString().data()));
      queries.push_back(ServerQuery(server, data["sql"].toString().data()));
    } else {
      // so we can report errors according to array index
      queries.push_back(ServerQuery(ServerDataPtr(), ""));
    }
  }

  Array ret;
  if (combine_result) {
    DBDataSet ds;
    map<int, string> errors;
    int affected = DBConn::parallelExecute(queries, ds, errors, max_thread,
                                           retry_query_on_fail,
                                           connect_timeout, read_timeout);
    output_dataset(ret, affected, ds, errors);
  } else {
    DBDataSetPtrVec dss(queries.size());
    for (unsigned int i = 0; i < dss.size(); i++) {
      dss[i] = DBDataSetPtr(new DBDataSet());
    }

    map<int, string> errors;
    int affected = DBConn::parallelExecute(queries, dss, errors, max_thread,
                                           retry_query_on_fail,
                                           connect_timeout, read_timeout);
    for (unsigned int i = 0; i < dss.size(); i++) {
      Array dsRet;
      output_dataset(dsRet, affected, *dss[i], errors);
      ret.append(dsRet);
    }
  }
  return ret;
}

Array f_fb_crossall_query(CStrRef sql, int max_thread /* = 50 */,
                          bool retry_query_on_fail /* = true */,
                          int connect_timeout /* = -1 */,
                          int read_timeout /* = -1 */,
                          bool timeout_in_ms /* = false */) {
  if (!timeout_in_ms) {
    if (connect_timeout > 0) connect_timeout *= 1000;
    if (read_timeout > 0) read_timeout *= 1000;
  }

  Array ret;
  // parameter checking
  if (!sql || !*sql) {
    ret.set("error", "empty SQL");
    return ret;
  }

  // security checking
  String ssql = StringUtil::ToLower(sql);
  if (ssql.find("where") < 0) {
    ret.set("error", "missing where clause");
    return ret;
  }
  if (ssql.find("select") < 0) {
    ret.set("error", "non-SELECT not supported");
    return ret;
  }

  // do it
  DBDataSet ds;
  map<int, string> errors;
  int affected = DBConn::parallelExecute(ssql.c_str(), ds, errors, max_thread,
                                         retry_query_on_fail,
                                         connect_timeout, read_timeout);
  output_dataset(ret, affected, ds, errors);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool f_fb_utf8ize(Variant input) {
  String s = input.toString();
  unsigned char *str = (unsigned char *)s.data();
  int len = s.size();
  int idx = 0;

  /*
    This works properly and makes the algorithm about twice as fast when
    processing very long single-byte strings, but it is only a little faster
    (~10%) when processing short single-byte strings and obviously not an
    optimization at all for multi-byte strings.

    int wide_len = (len >> 3);
    if (wide_len > 1) {

        unsigned long long *s = (unsigned long long *)str;
        unsigned long long *e = s + wide_len;

        do {
            if (//  This is a check for high bits in any byte, which means
                //  either junk or a multibyte character.
                (*s & ~0x7F7F7F7F7F7F7F7FULL) ||

                //  This is a check for a null byte. See:
                //      http://www-graphics.stanford.edu/~seander/bithacks.html
                ((*s - 0x0101010101010101ULL) & ~*s & 0x8080808080808080ULL)) {
                break;
            }
        } while (++s <= e);

        idx += ((s - (unsigned long long *)str) - 1) << 3;
    }

  */

  //  Scan the string for any multibyte characters.
  unsigned char c;
  for (       ; idx < len; ++idx) {
    c = str[idx];
    if (!c || c > 0x7F) {
      break;
    }
  }

  //  If we didn't encounter multibyte characters, the string is valid and
  //  we're done.
  if (idx == len) {
    return true;
  }

  //  We encountered multibyte characters. Parse the string optimistically,
  //  assuming it contains only valid UTF-8.
  int expect;
  int jj;
  for (       ; idx < len; ++idx) {
    c = str[idx];
    if (c && c < 0x80) {
      continue;
    } else if (c > 0xC1 && c < 0xE0) {
      expect = 1;
    } else if (c > 0xDF && c < 0xF0) {
      expect = 2;
    } else if (c > 0xEF && c < 0xF5) {
      expect = 3;
    } else {
      break;
    }

    for (jj = 0; jj < expect; ++jj) {
      if (++idx == len) {
        len -= (jj + 1);
        idx = len;
        input = s.substr(0, len);
        break;
      } else if (str[idx] < 0x80 || str[idx] > 0xBF) {
        idx -= (jj + 1);
        break;
      }
    }

    if (jj != expect) {
      break;
    }
  }

  //  If we made it through the whole string, it's valid UTF-8 with multibyte
  //  characters. We're done.
  if (idx == len) {
    return true;
  }

  //  We hit an invalid character sequence and need to remove invalid byte
  //  sequences from the string. We keep two pointers into the string, and
  //  copy from `src' to `dst'. `src' skips invalid subsequences while `dst'
  //  advacnes only on copy.
  unsigned char *src = str + idx;
  StringBuffer sb(len + 1);
  if (idx) {
    sb.append((char*)str, idx);
  }
  for (       ; src - str < len; ++src) {
    c = *src;
    if (c && c < 0x80) {
      sb.append(*src);
      continue;
    } else if (c > 0xC1 && c < 0xE0) {
      expect = 1;
    } else if (c > 0xDF && c < 0xF0) {
      expect = 2;
    } else if (c > 0xEF && c < 0xF5) {
      expect = 3;
    } else {
      continue;
    }

    for (jj = 0; jj < expect; ++jj) {
      if (++src - str == len) {
        input = sb.detach();
        return true;
      } else if (*src < 0x80 || *src > 0xBF) {
        src -= (jj + 1);
        break;
      }
    }

    if (jj == expect) {
      do {
        sb.append(*(src - expect));
      } while (expect--);
    }
  }

  input = sb.detach();
  return true;
}

bool f_fb_rename_function(CStrRef orig_func_name, CStrRef new_func_name) {
  if (orig_func_name.empty() || new_func_name.empty() ||
      strcasecmp(orig_func_name.data(), new_func_name.data()) == 0) {
    throw_invalid_argument("unable to rename %s", orig_func_name.data());
    return false;
  }

  if (!function_exists(orig_func_name)) {
    raise_warning("fb_rename_function(%s, %s) failed: %s does not exists!",
                  orig_func_name.data(), new_func_name.data(),
                  orig_func_name.data());
    return false;
  }

  if (function_exists(new_func_name)) {
    raise_warning("fb_rename_function(%s, %s) failed: %s already exists!",
                  orig_func_name.data(), new_func_name.data(),
                  new_func_name.data());
    return false;
  }

  hphp_const_char_imap<const char*> &funcs = get_renamed_functions();
  hphp_const_char_iset &ufuncs = get_unmapped_functions();

  char *new_key = new char[new_func_name.size() + 1];
  memcpy(new_key, new_func_name.data(), new_func_name.size() + 1);
  char *new_val = new char[orig_func_name.size() + 1];
  memcpy(new_val, orig_func_name.data(), orig_func_name.size() + 1);

  {
    hphp_const_char_imap<const char*>::iterator iter = funcs.find(new_key);
    if (iter != funcs.end()) {
      char *old_key = (char *)iter->first;
      char *old_val = (char *)iter->second;
      funcs.erase(iter);
      delete [] old_key;
      delete [] old_val;
    }

    // resolve to real name
    iter = funcs.find(new_val);
    if (iter != funcs.end()) {
      char *to_delete = new_val;

      int len = strlen(iter->second);
      new_val = new char[len + 1];
      memcpy(new_val, iter->second, len + 1);

      delete [] to_delete;
    }

    funcs[new_key] = new_val;
  }
  {
    hphp_const_char_iset::iterator iter = ufuncs.find(new_key);
    if (iter != ufuncs.end()) {
      char *old_key = (char *)*iter;
      ufuncs.erase(old_key);
      delete [] old_key;
    }

    iter = ufuncs.find(new_val);
    if (iter == ufuncs.end()) {
      new_val = new char[orig_func_name.size() + 1];
      memcpy(new_val, orig_func_name.data(), orig_func_name.size() + 1);
      ufuncs.insert(new_val);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// call_user_func extensions

Array f_fb_call_user_func_safe(int _argc, CVarRef function,
                               CArrRef _argv /* = null_array */) {
  return f_fb_call_user_func_array_safe(function, _argv);
}

Variant f_fb_call_user_func_safe_return(int _argc, CVarRef function,
                                        CVarRef def,
                                        CArrRef _argv /* = null_array */) {
  Variant call = function;
  if (!call.isString() && !call.isArray()) {
    call = function.toString();
  }
  if (f_is_callable(call)) {
    return f_call_user_func_array(call, _argv);
  }
  return def;
}

Array f_fb_call_user_func_array_safe(CVarRef function, CArrRef params) {
  Variant call = function;
  if (!call.isString() && !call.isArray()) {
    call = function.toString();
  }
  if (f_is_callable(call)) {
    return CREATE_VECTOR2(true, f_call_user_func_array(call, params));
  }
  return CREATE_VECTOR2(false, null);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_fb_get_code_coverage() {
  if (RuntimeOption::RecordCodeCoverage) {
    return Eval::CodeCoverage::Report();
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
