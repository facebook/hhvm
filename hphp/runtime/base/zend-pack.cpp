/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/req-tiny-vector.h"

#include <folly/Singleton.h>

#include <algorithm>

namespace HPHP {

#define INC_OUTPUTPOS(a,b)                                              \
  if ((a) < 0 || ((INT_MAX - outputpos)/((int)b)) < (a)) {              \
    raise_invalid_argument_warning                                      \
      ("Type %c: integer overflow in format string", code);             \
    return false;                                                       \
  }                                                                     \
  outputpos += (a)*(b);

///////////////////////////////////////////////////////////////////////////////

static folly::Singleton<ZendPack> zend_pack;
ZendPack* ZendPack::getInstance() {
  // ZendPack caches maps based on endianness
  // so only needs to be instantiated once
  return zend_pack.get();
}

ZendPack::ZendPack() {
  int machine_endian_check = 1;
  int64_t i;

  int64_t n = 2;
  for (i = 0; i < n; i++) {
    big_endian_2byte_map[i] = n - 1 - i;
    little_endian_2byte_map[i] = i;
  }

  n = 4;
  for (i = 0; i < n; i++) {
    big_endian_4byte_map[i] = n - 1 - i;
    little_endian_4byte_map[i] = i;
  }

  n = 8;
  for (i = 0; i < n; i++) {
    big_endian_8byte_map[i] = n - 1 - i;
    little_endian_8byte_map[i] = i;
  }

  machine_little_endian = ((char *)&machine_endian_check)[0];

  if (machine_little_endian) {
    /* Where to get lo to hi bytes from */
    byte_map[0] = 0;

    for (i = 0; i < (int)sizeof(int); i++) {
      int_map[i] = i;
    }

    std::copy(little_endian_2byte_map, little_endian_2byte_map + 2, machine_endian_2byte_map);
    std::copy(little_endian_4byte_map, little_endian_4byte_map + 4, machine_endian_4byte_map);
    std::copy(little_endian_8byte_map, little_endian_8byte_map + 8, machine_endian_8byte_map);
  } else {
    int64_t size = sizeof(int64_t);

    /* Where to get hi to lo bytes from */
    byte_map[0] = size - 1;

    for (i = 0; i < (int)sizeof(int); i++) {
      int_map[i] = size - (sizeof(int64_t) - i);
    }

    std::copy(big_endian_2byte_map, big_endian_2byte_map + 2, machine_endian_2byte_map);
    std::copy(big_endian_4byte_map, big_endian_4byte_map + 4, machine_endian_4byte_map);
    std::copy(big_endian_8byte_map, big_endian_8byte_map + 8, machine_endian_8byte_map);
  }
}

void packInt(const Variant& val, int64_t size, int64_t *map,
                    char *output) {
  int64_t n = val.toInt64();
  char *v = (char*)&n;
  for (int64_t i = 0; i < size; i++) {
    *output++ = v[map[i]];
  }
}

template <typename T, std::size_t N>
void packFloat(
    const Variant& val,
    int64_t (&map)[N],
    char *output) requires (N == sizeof(T)) {
  int64_t size = sizeof(T);
  T f = (T)val.toDouble();
  char *v = (char*)&f;
  for (int64_t i = 0; i < size; i++) {
    *output++ = v[map[i]];
  }
}

Variant ZendPack::pack(const String& fmt, const Array& argv) {
  /* Preprocess format into formatcodes and formatargs */
  req::TinyVector<char, 64> formatcodes; // up to 64 codes on the stack
  req::TinyVector<int, 64> formatargs;
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
        raise_invalid_argument_warning("Type %c: '*' ignored", code);
        arg = 1;
      }
      break;

      /* Always uses one arg */
    case 'a':
    case 'A':
    case 'h':
    case 'H':
    case 'Z':
      if (currentarg >= argc) {
        raise_invalid_argument_warning("Type %c: not enough arguments", code);
        return false;
      }

      if (arg < 0) {
        arg = argv[currentarg].toString().size();
        //add one, because Z is always NUL-terminated
        if (code == 'Z') {
          arg++;
        }
      }

      currentarg++;
      break;

      /* Use as many args as specified */
    case 'q':
    case 'Q':
    case 'J':
    case 'P':
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
    case 'f': /* float */
    case 'g': /* little endian float */
    case 'G': /* big endian float */
    case 'd': /* double */
    case 'e': /* little endian double */
    case 'E': /* big endian double */
      if (arg < 0) {
        arg = argc - currentarg;
      }

      currentarg += arg;

      if (currentarg > argc) {
        raise_invalid_argument_warning("Type %c: too few arguments", code);
        return false;
      }
      break;

    default:
      raise_invalid_argument_warning("Type %c: unknown format code", code);
      return false;
    }

    formatcodes.push_back(code);
    formatargs.push_back(arg);
  }

  if (currentarg < argc) {
    raise_invalid_argument_warning("%d arguments unused", (argc - currentarg));
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
    case 'Z':
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
    case 'q':
    case 'Q':
    case 'J':
    case 'P':
      INC_OUTPUTPOS(arg,8);    /* 64 bit per arg */
      break;

    case 'f': /* float */
    case 'g': /* little endian float */
    case 'G': /* big endian float */
      INC_OUTPUTPOS(arg,sizeof(float));
      break;

    case 'd': /* double */
    case 'e': /* little endian double */
    case 'E': /* big endian double */
      INC_OUTPUTPOS(arg,sizeof(double));
      break;

    case 'X':
      outputpos -= arg;

      if (outputpos < 0) {
        raise_invalid_argument_warning("Type %c: outside of string", code);
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

  String str = String(outputsize, ReserveString);
  char *output = str.mutableData();
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
    case 'Z': {
      int arg_cp = (code != 'Z') ? arg : std::max(0, arg - 1);
      memset(&output[outputpos], (code != 'A') ? '\0' : ' ', arg);
      val = argv[currentarg++].toString();
      s = val.c_str();
      slen = val.size();
      memcpy(&output[outputpos], s, (slen < arg_cp) ? slen : arg_cp);
      outputpos += arg;
    }
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
      if (arg > slen) {
        raise_invalid_argument_warning
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
          raise_invalid_argument_warning("Type %c: illegal hex digit %c", code, n);
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
        packInt(argv[currentarg++], 1, byte_map, &output[outputpos]);
        outputpos++;
      }
      break;

    case 's':
    case 'S':
    case 'n':
    case 'v': {
      int64_t *map = machine_endian_2byte_map;

      if (code == 'n') {
        map = big_endian_2byte_map;
      } else if (code == 'v') {
        map = little_endian_2byte_map;
      }

      while (arg-- > 0) {
        packInt(argv[currentarg++], 2, map, &output[outputpos]);
        outputpos += 2;
      }
      break;
    }

    case 'i':
    case 'I':
      while (arg-- > 0) {
        packInt(argv[currentarg++], sizeof(int), int_map, &output[outputpos]);
        outputpos += sizeof(int);
      }
      break;

    case 'l':
    case 'L':
    case 'N':
    case 'V': {
      int64_t *map = machine_endian_4byte_map;

      if (code == 'N') {
        map = big_endian_4byte_map;
      } else if (code == 'V') {
        map = little_endian_4byte_map;
      }

      while (arg-- > 0) {
        packInt(argv[currentarg++], 4, map, &output[outputpos]);
        outputpos += 4;
      }
      break;
    }

    case 'q':
    case 'Q':
    case 'J':
    case 'P': {
      int64_t *map = machine_endian_8byte_map;
      if (code == 'J') {
        map = big_endian_8byte_map;
      } else if (code == 'P') {
        map = little_endian_8byte_map;
      }

      while (arg-- > 0) {
        packInt(argv[currentarg++], 8, map, &output[outputpos]);
        outputpos += 8;
      }
      break;
    }

    case 'f':
    case 'g':
    case 'G': {
      auto map = &machine_endian_4byte_map;
      if (code == 'g') {
        map = &little_endian_4byte_map;
      } else if (code == 'G') {
        map = &big_endian_4byte_map;
      }
      while (arg-- > 0) {
        packFloat<float>(argv[currentarg++], *map, &output[outputpos]);
        outputpos += 4;
      }
      break;
    }

    case 'd':
    case 'e':
    case 'E': {
      auto map = &machine_endian_8byte_map;
      if (code == 'e') {
        map = &little_endian_8byte_map;
      } else if (code == 'E') {
        map = &big_endian_8byte_map;
      }
      while (arg-- > 0) {
        packFloat<double>(argv[currentarg++], *map, &output[outputpos]);
        outputpos += 8;
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

  str.setSize(outputpos);
  return str;
}

int64_t unpackInt(const char *data, int64_t size, int issigned,
                         int64_t *map) {
  int64_t result;
  char *cresult = (char *) &result;
  int i;

  result = issigned ? -1 : 0;

  for (i = 0; i < size; i++) {
    cresult[map[i]] = *data++;
  }

  return result;
}


template <typename T, std::size_t N>
double unpackFloat(
    const char *data,
    int64_t (&map)[N]) requires (N == sizeof(T)) {
  int64_t size = sizeof(T);
  T result;
  char *cresult = (char *) &result;
  int i;

  for (i = 0; i < size; i++) {
    cresult[map[i]] = *data++;
  }

  return (double)result;
}

Variant ZendPack::unpack(const String& fmt, const String& data) {
  const char *format = fmt.c_str();
  int formatlen = fmt.size();
  const char *input = data.c_str();
  int inputlen = data.size();
  int inputpos = 0;

  Array ret = Array::CreateDict();
  while (formatlen-- > 0) {
    char type = *(format++);
    int arg = 1, argb;
    const char *name;
    int namelen;
    int size=0;

    /* Handle format arguments if any */
    if (formatlen > 0) {
      char c = *format;

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
      if (arg < 0) {
        raise_invalid_argument_warning("Type %c: '*' ignored", type);
        arg = 1;
      }
      break;

    case '@':
      size = 0;
      break;

    case 'a':
    case 'A':
    case 'Z':
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

      /* Use machine dependent bytes of input */
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

      /* Use 8 bytes of input */
    case 'q':
    case 'Q':
    case 'J':
    case 'P':
      size = 8;
      break;
      /* Use sizeof(float) bytes of input */
    case 'f':
    case 'g':
    case 'G':
      size = sizeof(float);
      break;

      /* Use sizeof(double) bytes of input */
    case 'd':
    case 'e':
    case 'E':
      size = sizeof(double);
      break;

    default:
      raise_invalid_argument_warning("Invalid format type %c", type);
      return false;
    }

    /* Do actual unpacking */
    for (int i = 0; i != arg; i++ ) {
      /* Space for name + number, safe as namelen is ensured <= 200 */
      String n_str;

      if (namelen == 0) {
        n_str = String(i + 1);
      } else if (arg != 1) {
        /* Need to add element number to name */
        n_str = String(name, namelen, CopyString);
        n_str += String(i + 1);
      } else {
        /* Truncate name to next format code or end of string */
        n_str = String(name, namelen, CopyString);
      }

      int64_t n_int;
      const auto n_key = n_str.get()->isStrictlyInteger(n_int)
        ? Variant(n_int)
        : Variant(n_str);

      if (size != 0 && size != -1 && INT_MAX - size + 1 < inputpos) {
        raise_invalid_argument_warning("Type %c: integer overflow", type);
        inputpos = 0;
      }

      if ((inputpos + size) <= inputlen) {
        switch ((int) type) {
        case 'a':
        case 'A':
        case 'Z': {
          int len = inputlen - inputpos; /* Remaining string */

          /* If size was given take minimum of len and size */
          if ((size >= 0) && (len > size)) {
            len = size;
          }

          size = len;

          /* A will strip any trailing whitespace */
          if (type == 'A')
          {
            char padn = '\0'; char pads = ' '; char padt = '\t';
            char padc = '\r'; char padl = '\n';
            while (--len >= 0) {
               if (input[inputpos + len] != padn
                   && input[inputpos + len] != pads
                   && input[inputpos + len] != padt
                   && input[inputpos + len] != padc
                   && input[inputpos + len] != padl
               )
                       break;
            }
          }
          /* Remove everything after the first null */
          if (type=='Z') {
            int s;
            for (s=0 ; s < len ; s++) {
                     if (input[inputpos + s] == '\0')
                             break;
            }
            len = s;
          }

          /*only A is \0 terminated*/
          if (type=='A')
            len++;

          ret.set(n_key, String(input + inputpos, len, CopyString));
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
          buf = s.mutableData();

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
          ret.set(n_key, s);
          break;
        }

        case 'c':
        case 'C': {
          int issigned = (type == 'c') ? (input[inputpos] & 0x80) : 0;
          ret.set(n_key, unpackInt(&input[inputpos], 1, issigned, byte_map));
          break;
        }

        case 's':
        case 'S':
        case 'n':
        case 'v': {
          int issigned = 0;
          int64_t *map = machine_endian_2byte_map;

          if (type == 's') {
            issigned = input[inputpos + (machine_little_endian ? 1 : 0)] &
              0x80;
          } else if (type == 'n') {
            map = big_endian_2byte_map;
          } else if (type == 'v') {
            map = little_endian_2byte_map;
          }

          ret.set(n_key, unpackInt(&input[inputpos], 2, issigned, map));
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
                     (input[inputpos + machine_endian_4byte_map[3]]
                      & 0x80) == 0x80) {
            v = ~INT_MAX;
          }

          v |= unpackInt(&input[inputpos], sizeof(int), issigned, int_map);
          if (type == 'i') {
            ret.set(n_key, v);
          } else {
            uint64_t u64 = uint32_t(v);
            ret.set(n_key, u64);
          }
          break;
        }

        case 'l':
        case 'L':
        case 'N':
        case 'V': {
          int issigned = 0;
          int64_t *map = machine_endian_4byte_map;
          int64_t v = 0;

          if (type == 'l' || type == 'L') {
            issigned = input[inputpos + (machine_little_endian ? 3 : 0)]
              & 0x80;
          } else if (type == 'N') {
            issigned = input[inputpos] & 0x80;
            map = big_endian_4byte_map;
          } else if (type == 'V') {
            issigned = input[inputpos + 3] & 0x80;
            map = little_endian_4byte_map;
          }

          if (sizeof(int32_t) > 4 && issigned) {
            v = ~INT_MAX;
          }

          v |= unpackInt(&input[inputpos], 4, issigned, map);
          if (type == 'l') {
            ret.set(n_key, v);
          } else {
            uint64_t u64 = uint32_t(v);
            ret.set(n_key, u64);
          }
          break;
        }

        case 'q':
        case 'Q':
        case 'J':
        case 'P': {
          int issigned = 0;
          int64_t *map = machine_endian_8byte_map;
          int64_t v = 0;
          if (type == 'q' || type == 'Q') {
            issigned = input[inputpos + (machine_little_endian ? 7 : 0)] & 0x80;
          } else if (type == 'J') {
            issigned = input[inputpos] & 0x80;
            map = big_endian_8byte_map;
          } else if (type == 'P') {
            issigned = input[inputpos + 7] & 0x80;
            map = little_endian_8byte_map;
          }

          v = unpackInt(&input[inputpos], 8, issigned, map);

          if (type == 'q') {
            ret.set(n_key, v);
          } else {
            uint64_t u64 = uint64_t(v);
            ret.set(n_key, u64);
          }

          break;
        }

        case 'f': {/* float */
          ret.set(n_key, unpackFloat<float>(&input[inputpos], machine_endian_4byte_map));
          break;
        }

        case 'g': {/* little endian float */
          ret.set(n_key, unpackFloat<float>(&input[inputpos], little_endian_4byte_map));
          break;
        }

        case 'G': { /* big endian float */
          ret.set(n_key, unpackFloat<float>(&input[inputpos], big_endian_4byte_map));
          break;
        }

        case 'd': { /* double */
          ret.set(n_key, unpackFloat<double>(&input[inputpos], machine_endian_8byte_map));
          break;
        }

        case 'e': {/* little endian double */
          ret.set(n_key, unpackFloat<double>(&input[inputpos], little_endian_8byte_map));
          break;
        }

        case 'E': {/* big endian double */ 
          ret.set(n_key, unpackFloat<double>(&input[inputpos], big_endian_8byte_map));
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
              raise_invalid_argument_warning("Type %c: outside of string", type);
            }
          }
          break;

        case '@':
          if (arg <= inputlen) {
            inputpos = arg;
          } else {
            raise_invalid_argument_warning("Type %c: outside of string", type);
          }

          i = arg - 1;  /* Done, break out of for loop */
          break;
        }

        inputpos += size;
        if (inputpos < 0) {
          if (size != -1) { /* only print warning if not working with * */
            raise_invalid_argument_warning("Type %c: outside of string", type);
          }
          inputpos = 0;
        }
      } else if (arg < 0) {
        /* Reached end of input for '*' repeater */
        break;
      } else {
        raise_invalid_argument_warning
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
