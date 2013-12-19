/**
 * YAML parser and emitter PHP extension
 *
 * Copyright (c) 2007 Ryusuke SEKIYAMA. All rights reserved.
 * Copyright (c) 2009 Keynetics Inc. All rights reserved.
 * Copyright (c) 2012 Bryan Davis All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @package     php_yaml
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @author      Bryan Davis <bpd@keynetics.com>
 * @copyright   2007 Ryusuke SEKIYAMA
 * @copyright   2009 Keynetics Inc
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 * @version     SVN: $Id$
 */


#include "php_yaml.h"
#include "php_yaml_int.h"


/* {{{ local macros
 */
#define ts_skip_space() \
  while (ptr < end && (*ptr == ' ' || *ptr == '\t')) { \
    ptr++; \
  }

#define ts_skip_number() \
  while (ptr < end && *ptr >= '0' && *ptr <= '9') { \
    ptr++; \
  }

/* }}} */


/* {{{ local prototypes
 */
static long eval_sexagesimal_l(long lval, char *sg, char *eos);

static double eval_sexagesimal_d(double dval, char *sg, char *eos);

/* }}} */


/* {{{ detect_scalar_type(const char *, size_t, yaml_event_t)
 * Guess what datatype the scalar encodes
 */
const char *detect_scalar_type(const char *value, size_t length,
    const yaml_event_t *event)
{
  int flags = 0;
  long lval = 0;
  double dval = 0.0;

  /* is value a null? */
  if (0 == length || scalar_is_null(value, length, event)) {
    return YAML_NULL_TAG;
  }

  /* is value numeric? */
  flags = scalar_is_numeric(value, length, &lval, &dval, NULL);
  if (flags != Y_SCALAR_IS_NOT_NUMERIC) {
    return (flags & Y_SCALAR_IS_FLOAT) ? YAML_FLOAT_TAG : YAML_INT_TAG;
  }

  /* is value boolean? */
  flags = scalar_is_bool(value, length, event);
  if (-1 != flags) {
    return YAML_BOOL_TAG;
  }

  /* is value a timestamp? */
  if (scalar_is_timestamp(value, length)) {
    return YAML_TIMESTAMP_TAG;
  }

  /* no guess */
  return NULL;
}
/* }}} */


/* {{{ scalar_is_null(const char *,size_t,yaml_event_t)
 * Does this scalar encode a NULL value?
 *
 * specification is found at http://yaml.org/type/null.html.
 */
int
scalar_is_null(const char *value, size_t length,
    const yaml_event_t *event)
{
  if (NULL != event && event->data.scalar.quoted_implicit) {
    return 0;
  }

  if (NULL == event || event->data.scalar.plain_implicit) {
    if ((length == 1 && *value == '~') || length == 0 ||
        STR_EQ("NULL", value) || STR_EQ("Null", value) ||
        STR_EQ("null", value)) {
      return 1;
    }

  } else if (NULL != event && SCALAR_TAG_IS((*event), YAML_NULL_TAG)) {
    return 1;
  }

  return 0;
}
/* }}} */


/* {{{ scalar_is_bool(const char *,size_t,yaml_event_t)
 * Does this scalar encode a BOOL value?
 *
 * specification is found at http://yaml.org/type/bool.html.
 */
int
scalar_is_bool(const char *value, size_t length,
    const yaml_event_t *event)
{
  /* TODO: add ini setting to turn 'y'/'n' checks on/off */
  if (NULL == event || IS_NOT_QUOTED_OR_TAG_IS((*event), YAML_BOOL_TAG)) {
    if ((length == 1 && (*value == 'Y' || *value == 'y')) ||
        STR_EQ("YES", value) || STR_EQ("Yes", value) ||
        STR_EQ("yes", value) || STR_EQ("TRUE", value) ||
        STR_EQ("True", value) || STR_EQ("true", value) ||
        STR_EQ("ON", value) || STR_EQ("On", value) ||
        STR_EQ("on", value)) {
      return 1;
    }

    if ((length == 1 && (*value == 'N' || *value == 'n')) ||
        STR_EQ("NO", value) || STR_EQ("No", value) ||
        STR_EQ("no", value) || STR_EQ("FALSE", value) ||
        STR_EQ("False", value) || STR_EQ("false", value) ||
        STR_EQ("OFF", value) || STR_EQ("Off", value) ||
        STR_EQ("off", value)) {
      return 0;
    }

  } else if (NULL != event &&
      IS_NOT_IMPLICIT_AND_TAG_IS((*event), YAML_BOOL_TAG)) {
    if (0 == length || (1 == length && '0' == *value)) {
      return 0;
    } else {
      return 1;
    }
  }

  return -1;
}
/* }}} */


/* {{{ scalar_is_numeric()
 * Does this scalar encode a NUMERIC value?
 *
 * specification is found at http://yaml.org/type/float.html.
 * specification is found at http://yaml.org/type/int.html.
 */
int
scalar_is_numeric(const char *value, size_t length, long *lval,
    double *dval, char **str)
{
  const char *end = value + length;
  char *buf = { 0 }, *ptr = { 0 };
  int negative = 0;
  int type = 0;

  if (0 == length) {
    goto not_numeric;
  }

  /* trim */
  while (value < end && (*(end - 1) == ' ' || *(end - 1) == '\t')) {
    end--;
  }

  while (value < end && (*value == ' ' || *value == '\t')) {
    value++;
  }

  if (value == end) {
    goto not_numeric;
  }

  /* not a number */
  if (STR_EQ(".NAN", value) || STR_EQ(".NaN", value) ||
      STR_EQ(".nan", value)) {
    type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_NAN;
    goto finish;
  }

  /* sign */
  if (*value == '+') {
    value++;

  } else if (*value == '-') {
    negative = 1;
    value++;
  }

  if (value == end) {
    goto not_numeric;
  }

  /* infinity */
  if (STR_EQ(".INF", value) || STR_EQ(".Inf", value) ||
      STR_EQ(".inf", value)) {
    type = Y_SCALAR_IS_FLOAT;
    type |= (negative ? Y_SCALAR_IS_INFINITY_N : Y_SCALAR_IS_INFINITY_P);
    goto finish;
  }

  /* alloc */
  buf = (char *) emalloc(length + 3);
  ptr = buf;
  if (negative) {
    *ptr++ = '-';
  }

  /* parse */
  if (*value == '0') {
    *ptr++ = *value++;
    if (value == end) {
      goto return_zero;
    }

    if (*value == 'b') {
      /* binary integer */
      *ptr++ = *value++;
      if (value == end) {
        goto not_numeric;
      }

      while (value < end && (*value == '_' || *value == '0')) {
        value++;
      }

      if (value == end) {
        goto return_zero;
      }

      /* check the sequence */
      while (value < end) {
        if (*value == '_') {
          value++;

        } else if (*value == '0' || *value == '1') {
          *ptr++ = *value++;

        } else {
          goto not_numeric;
        }
      }

      type = Y_SCALAR_IS_INT | Y_SCALAR_IS_BINARY;

    } else if (*value == 'x') {
      /* hexadecimal integer */
      *ptr++ = *value++;

      if (value == end) {
        goto not_numeric;
      }

      while (value < end && (*value == '_' || *value == '0')) {
        value++;
      }

      if (value == end) {
        goto return_zero;
      }

      /* check the sequence */
      while (value < end) {
        if (*value == '_') {
          value++;

        } else if ((*value >= '0' && *value <= '9') ||
            (*value >= 'A' && *value <= 'F') ||
            (*value >= 'a' && *value <= 'f')) {
          *ptr++ = *value++;

        } else {
          goto not_numeric;
        }
      }

      type = Y_SCALAR_IS_INT | Y_SCALAR_IS_HEXADECIMAL;

    } else if (*value == '_' || (*value >= '0' && *value <= '7')) {
      /* octal integer */
      while (value < end) {
        if (*value == '_') {
          value++;

        } else if (*value >= '0' && *value <= '7') {
          *ptr++ = *value++;

        } else {
          goto not_numeric;
        }
      }

      type = Y_SCALAR_IS_INT | Y_SCALAR_IS_OCTAL;

    } else if (*value == '.') {
      goto check_float;

    } else {
      goto not_numeric;
    }

  } else if (*value >= '1' && *value <= '9') {
    /* integer */
    *ptr++ = *value++;
    while (value < end) {
      if (*value == '_' || *value == ',') {
        value++;

      } else if (*value >= '0' && *value <= '9') {
        *ptr++ = *value++;

      } else if (*value == ':') {
        goto check_sexa;

      } else if (*value == '.') {
        goto check_float;

      } else {
        goto not_numeric;

      }
    }

    type = Y_SCALAR_IS_INT | Y_SCALAR_IS_DECIMAL;

  } else if (*value == ':') {
    /* sexagecimal */

check_sexa:
    while (value < end - 2) {
      if (*value == '.') {
        type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_SEXAGECIMAL;
        goto check_float;
      }

      if (*value != ':') {
        goto not_numeric;
      }

      *ptr++ = *value++;
      if (*(value + 1) == ':') {
        if (*value >= '0' && *value <= '9') {
          *ptr++ = *value++;

        } else {
          goto not_numeric;
        }

      } else {
        if ((*value >= '0' && *value <= '5') &&
            (*(value + 1) >= '0' && *(value + 1) <= '9')) {
          *ptr++ = *value++;
          *ptr++ = *value++;

        } else {
          goto not_numeric;
        }
      }
    }

    if (*value == '.') {
      type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_SEXAGECIMAL;
      goto check_float;

    } else if (value == end) {
      type = Y_SCALAR_IS_INT | Y_SCALAR_IS_SEXAGECIMAL;

    } else {
      goto not_numeric;
    }

  } else if (*value == '.') {
    /* float */
    *ptr++ = '0';

check_float:
    *ptr++ = *value++;
    if (value == end) {
      /* don't treat strings ending with a period as numbers */
      /* mostly here to catch the degenerate case of `.` as input */
      goto not_numeric;
    }

    if (type == (Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_SEXAGECIMAL)) {
      /* sexagecimal float */
      while (value < end && (*(end - 1) == '_' || *(end - 1) == '0')) {
        end--;
      }

      if (value == end) {
        *ptr++ = '0';
      }

      while (value < end) {
        if (*value == '_') {
          value++;

        } else if (*value >= '0' && *value <= '9') {
          *ptr++ = *value++;

        } else {
          goto not_numeric;
        }
      }

    } else {
      /* decimal float */
      int is_exp = 0;
      while (value < end) {
        if (*value == '_') {
          value++;

        } else if (*value >= '0' && *value <= '9') {
          *ptr++ = *value++;

        } else if (*value == 'E' || *value == 'e') {
          /* exponential */
          is_exp = 1;

          *ptr++ = *value++;
          if (value == end || (*value != '+' && *value != '-')) {
            goto not_numeric;
          }

          *ptr++ = *value++;
          if (value == end || *value < '0' || *value > '9' ||
              (*value == '0' && value + 1 == end)) {
            goto not_numeric;
          }

          *ptr++ = *value++;
          while (value < end) {
            if (*value >= '0' && *value <= '9') {
              *ptr++ = *value++;

            } else {
              goto not_numeric;
            }
          }

        } else {
          goto not_numeric;
        }
      }

      /* trim */
      if (!is_exp) {
        while (*(ptr - 1) == '0') {
          ptr--;
        }

        if (*(ptr - 1) == '.') {
          *ptr++ = '0';
        }
      }

      type = Y_SCALAR_IS_FLOAT | Y_SCALAR_IS_DECIMAL;
    }

  } else {
    goto not_numeric;
  }

  /* terminate */
  *ptr = '\0';

finish:
  /* convert & assign */
  if ((type & Y_SCALAR_IS_INT) && lval != NULL) {
    switch (type & Y_SCALAR_FORMAT_MASK) {
    case Y_SCALAR_IS_BINARY:
      ptr = buf + 2;
      if (*ptr == 'b') {
        ptr++;
      }

      *lval = strtol(ptr, (char **) NULL, 2);
      if (*buf == '-') {
        *lval *= -1L;
      }
      break;

    case Y_SCALAR_IS_OCTAL:
      *lval = strtol(buf, (char **) NULL, 8);
      break;

    case Y_SCALAR_IS_HEXADECIMAL:
      *lval = strtol(buf, (char **) NULL, 16);
      break;

    case Y_SCALAR_IS_SEXAGECIMAL:
      *lval = eval_sexagesimal_l(0, buf, ptr);
      if (*buf == '-') {
        *lval *= -1L;
      }
      break;

    default:
      *lval = atol(buf);
      break;
    }

  } else if ((type & Y_SCALAR_IS_FLOAT) && dval != NULL) {
    switch (type & Y_SCALAR_FORMAT_MASK) {
    case Y_SCALAR_IS_SEXAGECIMAL:
      *dval = eval_sexagesimal_d(0.0, buf, ptr);
      if (*buf == '-') {
        *dval *= -1.0;
      }
      break;

    case Y_SCALAR_IS_INFINITY_P:
      *dval = php_get_inf();
      break;

    case Y_SCALAR_IS_INFINITY_N:
      *dval = -php_get_inf();
      break;

    case Y_SCALAR_IS_NAN:
      *dval = php_get_nan();
      break;

    default:
      *dval = atof(buf);
      break;
    }
  }

  if (buf != NULL) {
    if (str != NULL) {
      *str = buf;

    } else {
      efree(buf);
    }
  }

  /* return */
  return type;


return_zero:
  if (lval != NULL) {
    *lval = 0;
  }

  if (dval != NULL) {
    *dval = 0.0;
  }

  if (buf != NULL) {
    efree(buf);
  }

  return (Y_SCALAR_IS_INT | Y_SCALAR_IS_ZERO);


not_numeric:
  if (buf != NULL) {
    efree(buf);
  }

  return Y_SCALAR_IS_NOT_NUMERIC;
}
/* }}} */


/* {{{ scalar_is_timestamp(const char *,size_t)
 * Does this scalar encode a TIMESTAMP value?
 *
 * specification is found at http://yaml.org/type/timestamp.html.
 */
int scalar_is_timestamp(const char *value, size_t length)
{
  const char *ptr = value;
  const char *end = value + length;
  const char *pos1, *pos2;

  /* skip leading space */
  ts_skip_space();

  /* check 4 digit year and separator */
  pos1 = pos2 = ptr;
  ts_skip_number();
  if (ptr == pos1 || ptr == end || ptr - pos2 != 4 || *ptr != '-') {
    return 0;
  }

  /* check 1 or 2 month and separator */
  pos2 = ++ptr;
  ts_skip_number();
  if (ptr == pos2 || ptr == end || ptr - pos2 > 2 || *ptr != '-') {
    return 0;
  }

  /* check 1 or 2 digit day */
  pos2 = ++ptr;
  ts_skip_number();
  if (ptr == pos2 || ptr - pos2 > 2) {
    return 0;
  }

  /* check separator */
  pos2 = ptr;
  if (ptr == end) {
    /* date only format requires YYYY-MM-DD */
    return (pos2 - pos1 == 10) ? 1 : 0;
  }

  /* time separator is T or whitespace */
  if (*ptr == 'T' || *ptr == 't') {
    ptr++;

  } else {
    ts_skip_space();
  }

  /* check 1 or 2 digit hour and separator */
  pos1 = ptr;
  ts_skip_number();
  if (ptr == pos1 || ptr == end || ptr - pos1 > 2 || *ptr != ':') {
    return 0;
  }

  /* check 2 digit minute and separator */
  pos1 = ++ptr;
  ts_skip_number();
  if (ptr == end || ptr - pos1 != 2 || *ptr != ':') {
    return 0;
  }

  /* check 2 digit second */
  pos1 = ++ptr;
  ts_skip_number();
  if (ptr == end) {
    return (ptr - pos1 == 2) ? 1 : 0;
  }

  /* check optional fraction */
  if (*ptr == '.') {
    ptr++;
    ts_skip_number();
  }

  /* skip optional separator space */
  ts_skip_space();
  if (ptr == end) {
    return 1;
  }

  /* check time zone */
  if (*ptr == 'Z') {
    ptr++;
    ts_skip_space();
    return (ptr == end) ? 1 : 0;
  }

  /* check time zone offset sign */
  if (*ptr != '+' && *ptr != '-') {
    return 0;
  }

  /* check 1 or 2 digit time zone hour */
  pos1 = ++ptr;
  ts_skip_number();
  if (ptr == pos1 || ptr - pos1 == 3 || ptr - pos1 > 4) {
    return 0;
  }

  if (ptr == end) {
    return 1;
  }

  /* optional time zone minute */
  if (*ptr != ':') {
    return 0;
  }

  pos1 = ++ptr;
  ts_skip_number();

  if (ptr - pos1 != 2) {
    return 0;
  }

  /* skip following space */
  ts_skip_space();
  return (ptr == end) ? 1 : 0;
}
/* }}} */


/* {{{ eval_sexagesimal_l()
 * Convert a base 60 number to a long
 */
static long eval_sexagesimal_l(long lval, char *sg, char *eos)
{
  char *ep;

  while (sg < eos && (*sg < '0' || *sg > '9')) {
    sg++;
  }

  ep = sg;
  while (ep < eos && *ep >= '0' && *ep <= '9') {
    ep++;
  }

  if (sg == eos) {
    return lval;
  }

  return eval_sexagesimal_l(
      lval * 60 + strtol(sg, (char **) NULL, 10), ep, eos);
}
/* }}} */


/* {{{ eval_sexagesimal_d()
 * Convert a base 60 number to a double
 */
static double eval_sexagesimal_d(double dval, char *sg, char *eos)
{
  char *ep;

  while (sg < eos && *sg != '.' && (*sg < '0' || *sg > '9')) {
    sg++;
  }

  ep = sg;
  while (ep < eos && *ep >= '0' && *ep <= '9') {
    ep++;
  }

  if (sg == eos || *sg == '.') {
    return dval;
  }

  return eval_sexagesimal_d(
      dval * 60.0 + strtod(sg, (char **) NULL), ep, eos);
}
/* }}} */



