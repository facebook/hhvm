#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_operators.h"

/**
 * Checks whether the string "str" with length "length" is numeric. The value
 * of allow_errors determines whether it's required to be entirely numeric, or
 * just its prefix. Leading whitespace is allowed.
 *
 * The function returns 0 if the string did not contain a valid number; IS_LONG
 * if it contained a number that fits within the range of a long; or IS_DOUBLE
 * if the number was out of long range or contained a decimal point/exponent.
 * The number's value is returned into the respective pointer, *lval or *dval,
 * if that pointer is not NULL.
 *
 * This variant also gives information if a string that represents an integer
 * could not be represented as such due to overflow. It writes 1 to oflow_info
 * if the integer is larger than LONG_MAX and -1 if it's smaller than LONG_MIN.
 */
zend_uchar is_numeric_string_ex(const char *str, int length, long *lval, double *dval, int allow_errors, int *oflow_info) {
  const char *ptr;
  int base = 10, digits = 0, dp_or_e = 0;
  double local_dval;
  zend_uchar type;

  if (!length) {
    return 0;
  }

  if (oflow_info != NULL) {
    *oflow_info = 0;
  }

  /* Skip any whitespace
   * This is much faster than the isspace() function */
  while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r' || *str == '\v' || *str == '\f') {
    str++;
    length--;
  }
  ptr = str;

  if (*ptr == '-' || *ptr == '+') {
    ptr++;
  }

  if (ZEND_IS_DIGIT(*ptr)) {
    /* Handle hex numbers
     * str is used instead of ptr to disallow signs and keep old behavior */
    if (length > 2 && *str == '0' && (str[1] == 'x' || str[1] == 'X')) {
      base = 16;
      ptr += 2;
    }

    /* Skip any leading 0s */
    while (*ptr == '0') {
      ptr++;
    }

    /* Count the number of digits. If a decimal point/exponent is found,
     * it's a double. Otherwise, if there's a dval or no need to check for
     * a full match, stop when there are too many digits for a long */
    for (type = IS_LONG; !(digits >= MAX_LENGTH_OF_LONG && (dval || allow_errors == 1)); digits++, ptr++) {
check_digits:
      if (ZEND_IS_DIGIT(*ptr) || (base == 16 && ZEND_IS_XDIGIT(*ptr))) {
        continue;
      } else if (base == 10) {
        if (*ptr == '.' && dp_or_e < 1) {
          goto process_double;
        } else if ((*ptr == 'e' || *ptr == 'E') && dp_or_e < 2) {
          const char *e = ptr + 1;

          if (*e == '-' || *e == '+') {
            ptr = e++;
          }
          if (ZEND_IS_DIGIT(*e)) {
            goto process_double;
          }
        }
      }

      break;
    }

    if (base == 10) {
      if (digits >= MAX_LENGTH_OF_LONG) {
        if (oflow_info != NULL) {
          *oflow_info = *str == '-' ? -1 : 1;
        }
        dp_or_e = -1;
        goto process_double;
      }
    } else if (!(digits < SIZEOF_LONG * 2 || (digits == SIZEOF_LONG * 2 && ptr[-digits] <= '7'))) {
      if (dval) {
        local_dval = HPHP::zend_hex_strtod(str, &ptr);
      }
      if (oflow_info != NULL) {
        *oflow_info = 1;
      }
      type = IS_DOUBLE;
    }
  } else if (*ptr == '.' && ZEND_IS_DIGIT(ptr[1])) {
process_double:
    type = IS_DOUBLE;

    /* If there's a dval, do the conversion; else continue checking
     * the digits if we need to check for a full match */
    if (dval) {
      local_dval = HPHP::zend_strtod(str, &ptr);
    } else if (allow_errors != 1 && dp_or_e != -1) {
      dp_or_e = (*ptr++ == '.') ? 1 : 2;
      goto check_digits;
    }
  } else {
    return 0;
  }

  if (ptr != str + length) {
    if (!allow_errors) {
      return 0;
    }
    if (allow_errors == -1) {
      zend_error(E_NOTICE, "A non well formed numeric value encountered");
    }
  }

  if (type == IS_LONG) {
    if (digits == MAX_LENGTH_OF_LONG - 1) {
      int cmp = strcmp(&ptr[-digits], long_min_digits);

      if (!(cmp < 0 || (cmp == 0 && *str == '-'))) {
        if (dval) {
          *dval = HPHP::zend_strtod(str, NULL);
        }
        if (oflow_info != NULL) {
          *oflow_info = *str == '-' ? -1 : 1;
        }

        return IS_DOUBLE;
      }
    }

    if (lval) {
      *lval = strtol(str, NULL, base);
    }

    return IS_LONG;
  } else {
    if (dval) {
      *dval = local_dval;
    }

    return IS_DOUBLE;
  }
}

HPHP::DataType& Z_TYPE(const zval& z) {
  HPHP::DataType* dt = &const_cast<zval*>(&z)->tv()->m_type;
  assert(*dt != HPHP::KindOfRef);
  if (*dt == HPHP::KindOfStaticString) {
    *dt = IS_STRING;
  }
  return *dt;
}

