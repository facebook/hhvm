/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/zend-pack.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"

namespace HPHP {

#define INC_OUTPUTPOS(a,b)                                              \
  if ((a) < 0 || ((INT_MAX - outputpos)/((int)b)) < (a)) {              \
    throw_invalid_argument                                              \
      ("Type %c: integer overflow in format string", code);             \
    return false;                                                       \
  }                                                                     \
  outputpos += (a)*(b);

///////////////////////////////////////////////////////////////////////////////

ZendPack::ZendPack() {
  int machine_endian_check = 1;
  int i;

  machine_little_endian = ((char *)&machine_endian_check)[0];

  if (machine_little_endian) {
    /* Where to get lo to hi bytes from */
    byte_map[0] = 0;

    for (i = 0; i < (int)sizeof(int); i++) {
      int_map[i] = i;
    }

    machine_endian_short_map[0] = 0;
    machine_endian_short_map[1] = 1;
    big_endian_short_map[0] = 1;
    big_endian_short_map[1] = 0;
    little_endian_short_map[0] = 0;
    little_endian_short_map[1] = 1;

    machine_endian_int32_map[0] = 0;
    machine_endian_int32_map[1] = 1;
    machine_endian_int32_map[2] = 2;
    machine_endian_int32_map[3] = 3;
    big_endian_int32_map[0] = 3;
    big_endian_int32_map[1] = 2;
    big_endian_int32_map[2] = 1;
    big_endian_int32_map[3] = 0;
    little_endian_int32_map[0] = 0;
    little_endian_int32_map[1] = 1;
    little_endian_int32_map[2] = 2;
    little_endian_int32_map[3] = 3;
  } else {
    int size = sizeof(int32_t);

    /* Where to get hi to lo bytes from */
    byte_map[0] = size - 1;

    for (i = 0; i < (int)sizeof(int); i++) {
      int_map[i] = size - (sizeof(int) - i);
    }

    machine_endian_short_map[0] = size - 2;
    machine_endian_short_map[1] = size - 1;
    big_endian_short_map[0] = size - 2;
    big_endian_short_map[1] = size - 1;
    little_endian_short_map[0] = size - 1;
    little_endian_short_map[1] = size - 2;

    machine_endian_int32_map[0] = size - 4;
    machine_endian_int32_map[1] = size - 3;
    machine_endian_int32_map[2] = size - 2;
    machine_endian_int32_map[3] = size - 1;
    big_endian_int32_map[0] = size - 4;
    big_endian_int32_map[1] = size - 3;
    big_endian_int32_map[2] = size - 2;
    big_endian_int32_map[3] = size - 1;
    little_endian_int32_map[0] = size - 1;
    little_endian_int32_map[1] = size - 2;
    little_endian_int32_map[2] = size - 3;
    little_endian_int32_map[3] = size - 4;
  }
}

void ZendPack::pack(CVarRef val, int size, int *map, char *output) {
  int32_t n = val.toInt32();
  char *v = (char*)&n;
  for (int i = 0; i < size; i++) {
    *output++ = v[map[i]];
  }
}

Variant ZendPack::pack(const String& fmt, CArrRef argv) {
  /* Preprocess format into formatcodes and formatargs */
  vector<char> formatcodes;
  vector<int> formatargs;
  int argc = argv.size();

  const char *format = fmt.c_str();
  int formatlen = fmt.size();
  int currentarg = 0;
  for (int i = 0; i < formatlen; ) {
    char code = format[i++];
    int arg = 1;

    /* Handle format arguments if any */
    if (i < formatlen) {
      char c = format[i];

      if (c == '*') {
        arg = -1;
        i++;
      }
      else if (c >= '0' && c <= '9') {
        arg = atoi(&format[i]);

        while (format[i] >= '0' && format[i] <= '9' && i < formatlen) {
          i++;
        }
      }
    }

    /* Handle special arg '*' for all codes and check argv overflows */
    switch ((int) code) {
      /* Never uses any args */
    case 'x':
    case 'X':
    case '@':
      if (arg < 0) {
        throw_invalid_argument("Type %c: '*' ignored", code);
        arg = 1;
      }
      break;

      /* Always uses one arg */
    case 'a':
    case 'A':
    case 'h':
    case 'H':
      if (currentarg >= argc) {
        throw_invalid_argument("Type %c: not enough arguments", code);
        return false;
      }

      if (arg < 0) {
        arg = argv[currentarg].toString().size();
      }

      currentarg++;
      break;

      /* Use as many args as specified */
    case 'c':
    case 'C':
    case 's':
    case 'S':
    case 'i':
    case 'I':
    case 'l':
    case 'L':
    case 'n':
    case 'N':
    case 'v':
    case 'V':
    case 'f':
    case 'd':
      if (arg < 0) {
        arg = argc - currentarg;
      }

      currentarg += arg;

      if (currentarg > argc) {
        throw_invalid_argument("Type %c: too few arguments", code);
        return false;
      }
      break;

    default:
      throw_invalid_argument("Type %c: unknown format code", code);
      return false;
    }

    formatcodes.push_back(code);
    formatargs.push_back(arg);
  }

  if (currentarg < argc) {
    throw_invalid_argument("%d arguments unused", (argc - currentarg));
  }

  int outputpos = 0, outputsize = 0;
  /* Calculate output length and upper bound while processing*/
  for (int i = 0; i < (int)formatcodes.size(); i++) {
    int code = (int) formatcodes[i];
    int arg = formatargs[i];

    switch ((int) code) {
    case 'h':
    case 'H':
      INC_OUTPUTPOS((arg + (arg % 2)) / 2,1);  /* 4 bit per arg */
      break;

    case 'a':
    case 'A':
    case 'c':
    case 'C':
    case 'x':
      INC_OUTPUTPOS(arg,1);    /* 8 bit per arg */
      break;

    case 's':
    case 'S':
    case 'n':
    case 'v':
      INC_OUTPUTPOS(arg,2);    /* 16 bit per arg */
      break;

    case 'i':
    case 'I':
      INC_OUTPUTPOS(arg,sizeof(int));
      break;

    case 'l':
    case 'L':
    case 'N':
    case 'V':
      INC_OUTPUTPOS(arg,4);    /* 32 bit per arg */
      break;

    case 'f':
      INC_OUTPUTPOS(arg,sizeof(float));
      break;

    case 'd':
      INC_OUTPUTPOS(arg,sizeof(double));
      break;

    case 'X':
      outputpos -= arg;

      if (outputpos < 0) {
        throw_invalid_argument("Type %c: outside of string", code);
        outputpos = 0;
      }
      break;

    case '@':
      outputpos = arg;
      break;
    }

    if (outputsize < outputpos) {
      outputsize = outputpos;
    }
  }

  String s = String(outputsize, ReserveString);
  char *output = s.bufferSlice().ptr;
  outputpos = 0;
  currentarg = 0;

  /* Do actual packing */
  for (int i = 0; i < (int)formatcodes.size(); i++) {
    int code = (int) formatcodes[i];
    int arg = formatargs[i];
    String val;
    const char *s;
    int slen;

    switch ((int) code) {
    case 'a':
    case 'A':
      memset(&output[outputpos], (code == 'a') ? '\0' : ' ', arg);
      val = argv[currentarg++].toString();
      s = val.c_str();
      slen = val.size();
      memcpy(&output[outputpos], s, (slen < arg) ? slen : arg);
      outputpos += arg;
      break;

    case 'h':
    case 'H': {
      int nibbleshift = (code == 'h') ? 0 : 4;
      int first = 1;
      const char *v;

      val = argv[currentarg++].toString();
      v = val.data();
      slen = val.size();
      outputpos--;
      if(arg > slen) {
        throw_invalid_argument
          ("Type %c: not enough characters in string", code);
        arg = slen;
      }

      while (arg-- > 0) {
        char n = *v++;

        if (n >= '0' && n <= '9') {
          n -= '0';
        } else if (n >= 'A' && n <= 'F') {
          n -= ('A' - 10);
        } else if (n >= 'a' && n <= 'f') {
          n -= ('a' - 10);
        } else {
          throw_invalid_argument("Type %c: illegal hex digit %c", code, n);
          n = 0;
        }

        if (first--) {
          output[++outputpos] = 0;
        } else {
          first = 1;
        }

        output[outputpos] |= (n << nibbleshift);
        nibbleshift = (nibbleshift + 4) & 7;
      }

      outputpos++;
      break;
    }

    case 'c':
    case 'C':
      while (arg-- > 0) {
        pack(argv[currentarg++], 1, byte_map, &output[outputpos]);
        outputpos++;
      }
      break;

    case 's':
    case 'S':
    case 'n':
    case 'v': {
      int *map = machine_endian_short_map;

      if (code == 'n') {
        map = big_endian_short_map;
      } else if (code == 'v') {
        map = little_endian_short_map;
      }

      while (arg-- > 0) {
        pack(argv[currentarg++], 2, map, &output[outputpos]);
        outputpos += 2;
      }
      break;
    }

    case 'i':
    case 'I':
      while (arg-- > 0) {
        pack(argv[currentarg++], sizeof(int), int_map, &output[outputpos]);
        outputpos += sizeof(int);
      }
      break;

    case 'l':
    case 'L':
    case 'N':
    case 'V': {
      int *map = machine_endian_int32_map;

      if (code == 'N') {
        map = big_endian_int32_map;
      } else if (code == 'V') {
        map = little_endian_int32_map;
      }

      while (arg-- > 0) {
        pack(argv[currentarg++], 4, map, &output[outputpos]);
        outputpos += 4;
      }
      break;
    }

    case 'f': {
      float v;

      while (arg-- > 0) {
        v = argv[currentarg++].toDouble();
        memcpy(&output[outputpos], &v, sizeof(v));
        outputpos += sizeof(v);
      }
      break;
    }

    case 'd': {
      double v;

      while (arg-- > 0) {
        v = argv[currentarg++].toDouble();
        memcpy(&output[outputpos], &v, sizeof(v));
        outputpos += sizeof(v);
      }
      break;
    }

    case 'x':
      memset(&output[outputpos], '\0', arg);
      outputpos += arg;
      break;

    case 'X':
      outputpos -= arg;

      if (outputpos < 0) {
        outputpos = 0;
      }
      break;

    case '@':
      if (arg > outputpos) {
        memset(&output[outputpos], '\0', arg - outputpos);
      }
      outputpos = arg;
      break;
    }
  }

  return s.setSize(outputpos);
}

int32_t ZendPack::unpack(const char *data, int size, int issigned, int *map) {
  int32_t result;
  char *cresult = (char *) &result;
  int i;

  result = issigned ? -1 : 0;

  for (i = 0; i < size; i++) {
    cresult[map[i]] = *data++;
  }

  return result;
}

Variant ZendPack::unpack(const String& fmt, const String& data) {
  const char *format = fmt.c_str();
  int formatlen = fmt.size();
  const char *input = data.c_str();
  int inputlen = data.size();
  int inputpos = 0;

  Array ret;
  while (formatlen-- > 0) {
    char type = *(format++);
    char c;
    int arg = 1, argb;
    const char *name;
    int namelen;
    int size=0;

    /* Handle format arguments if any */
    if (formatlen > 0) {
      c = *format;

      if (c >= '0' && c <= '9') {
        arg = atoi(format);

        while (formatlen > 0 && *format >= '0' && *format <= '9') {
          format++;
          formatlen--;
        }
      } else if (c == '*') {
        arg = -1;
        format++;
        formatlen--;
      }
    }

    /* Get of new value in array */
    name = format;
    argb = arg;

    while (formatlen > 0 && *format != '/') {
      formatlen--;
      format++;
    }

    namelen = format - name;

    if (namelen > 200)
      namelen = 200;

    switch ((int) type) {
      /* Never use any input */
    case 'X':
      size = -1;
      break;

    case '@':
      size = 0;
      break;

    case 'a':
    case 'A':
      size = arg;
      arg = 1;
      break;

    case 'h':
    case 'H':
      size = (arg > 0) ? (arg + (arg % 2)) / 2 : arg;
      arg = 1;
      break;

      /* Use 1 byte of input */
    case 'c':
    case 'C':
    case 'x':
      size = 1;
      break;

      /* Use 2 bytes of input */
    case 's':
    case 'S':
    case 'n':
    case 'v':
      size = 2;
      break;

      /* Use sizeof(int) bytes of input */
    case 'i':
    case 'I':
      size = sizeof(int);
      break;

      /* Use 4 bytes of input */
    case 'l':
    case 'L':
    case 'N':
    case 'V':
      size = 4;
      break;

      /* Use sizeof(float) bytes of input */
    case 'f':
      size = sizeof(float);
      break;

      /* Use sizeof(double) bytes of input */
    case 'd':
      size = sizeof(double);
      break;

    default:
      throw_invalid_argument("Invalid format type %c", type);
      return false;
    }

    /* Do actual unpacking */
    for (int i = 0; i != arg; i++ ) {
      /* Space for name + number, safe as namelen is ensured <= 200 */
      char n[256];

      if (arg != 1 || namelen == 0) {
        /* Need to add element number to name */
        snprintf(n, sizeof(n), "%.*s%d", namelen, name, i + 1);
      } else {
        /* Truncate name to next format code or end of string */
        snprintf(n, sizeof(n), "%.*s", namelen, name);
      }

      if (size != 0 && size != -1 && INT_MAX - size + 1 < inputpos) {
        throw_invalid_argument("Type %c: integer overflow", type);
        inputpos = 0;
      }

      if ((inputpos + size) <= inputlen) {
        switch ((int) type) {
        case 'a':
        case 'A': {
          char pad = (type == 'a') ? '\0' : ' ';
          int len = inputlen - inputpos; /* Remaining string */

          /* If size was given take minimum of len and size */
          if ((size >= 0) && (len > size)) {
            len = size;
          }

          size = len;

          /* Remove padding chars from unpacked data */
          while (--len >= 0) {
            if (input[inputpos + len] != pad)
              break;
          }

          ret.set(String(n, CopyString),
                  String(input + inputpos, len + 1, CopyString));
          break;
        }

        case 'h':
        case 'H': {
          int len = (inputlen - inputpos) * 2;  /* Remaining */
          int nibbleshift = (type == 'h') ? 0 : 4;
          int first = 1;
          char *buf;
          int ipos, opos;

          /* If size was given take minimum of len and size */
          if (size >= 0 && len > (size * 2)) {
            len = size * 2;
          }

          if (argb > 0) {
            len -= argb % 2;
          }

          String s = String(len, ReserveString);
          buf = s.bufferSlice().ptr;

          for (ipos = opos = 0; opos < len; opos++) {
            char c = (input[inputpos + ipos] >> nibbleshift) & 0xf;

            if (c < 10) {
              c += '0';
            } else {
              c += 'a' - 10;
            }

            buf[opos] = c;
            nibbleshift = (nibbleshift + 4) & 7;

            if (first-- == 0) {
              ipos++;
              first = 1;
            }
          }

          s.setSize(len);
          ret.set(String(n, CopyString), s);
          break;
        }

        case 'c':
        case 'C': {
          int issigned = (type == 'c') ? (input[inputpos] & 0x80) : 0;
          ret.set(String(n, CopyString),
                  unpack(&input[inputpos], 1, issigned, byte_map));
          break;
        }

        case 's':
        case 'S':
        case 'n':
        case 'v': {
          int issigned = 0;
          int *map = machine_endian_short_map;

          if (type == 's') {
            issigned = input[inputpos + (machine_little_endian ? 1 : 0)] &
              0x80;
          } else if (type == 'n') {
            map = big_endian_short_map;
          } else if (type == 'v') {
            map = little_endian_short_map;
          }

          ret.set(String(n, CopyString),
                  unpack(&input[inputpos], 2, issigned, map));
          break;
        }

        case 'i':
        case 'I': {
          int32_t v = 0;
          int issigned = 0;

          if (type == 'i') {
            issigned = input[inputpos + (machine_little_endian ?
                                         (sizeof(int) - 1) : 0)] & 0x80;
          } else if (sizeof(int32_t) > 4 &&
                     (input[inputpos + machine_endian_int32_map[3]]
                      & 0x80) == 0x80) {
            v = ~INT_MAX;
          }

          v |= unpack(&input[inputpos], sizeof(int), issigned, int_map);
          if (type == 'i') {
            ret.set(String(n, CopyString), v);
          } else {
            uint64_t u64 = uint32_t(v);
            ret.set(String(n, CopyString), u64);
          }
          break;
        }

        case 'l':
        case 'L':
        case 'N':
        case 'V': {
          int issigned = 0;
          int *map = machine_endian_int32_map;
          int32_t v = 0;

          if (type == 'l' || type == 'L') {
            issigned = input[inputpos + (machine_little_endian ? 3 : 0)]
              & 0x80;
          } else if (type == 'N') {
            issigned = input[inputpos] & 0x80;
            map = big_endian_int32_map;
          } else if (type == 'V') {
            issigned = input[inputpos + 3] & 0x80;
            map = little_endian_int32_map;
          }

          if (sizeof(int32_t) > 4 && issigned) {
            v = ~INT_MAX;
          }

          v |= unpack(&input[inputpos], 4, issigned, map);
          if (type == 'l') {
            ret.set(String(n, CopyString), v);
          } else {
            uint64_t u64 = uint32_t(v);
            ret.set(String(n, CopyString), u64);
          }
          break;
        }

        case 'f': {
          float v;

          memcpy(&v, &input[inputpos], sizeof(float));
          ret.set(String(n, CopyString), (double)v);
          break;
        }

        case 'd': {
          double v;

          memcpy(&v, &input[inputpos], sizeof(double));
          ret.set(String(n, CopyString), v);
          break;
        }

        case 'x':
          /* Do nothing with input, just skip it */
          break;

        case 'X':
          if (inputpos < size) {
            inputpos = -size;
            i = arg - 1;    /* Break out of for loop */

            if (arg >= 0) {
              throw_invalid_argument("Type %c: outside of string", type);
            }
          }
          break;

        case '@':
          if (arg <= inputlen) {
            inputpos = arg;
          } else {
            throw_invalid_argument("Type %c: outside of string", type);
          }

          i = arg - 1;  /* Done, break out of for loop */
          break;
        }

        inputpos += size;
        if (inputpos < 0) {
          if (size != -1) { /* only print warning if not working with * */
            throw_invalid_argument("Type %c: outside of string", type);
          }
          inputpos = 0;
        }
      } else if (arg < 0) {
        /* Reached end of input for '*' repeater */
        break;
      } else {
        throw_invalid_argument
          ("Type %c: not enough input, need %d, have %d",
           type, size, inputlen - inputpos);
        return false;
      }
    }

    formatlen--; /* Skip '/' separator, does no harm if inputlen == 0 */
    format++;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
