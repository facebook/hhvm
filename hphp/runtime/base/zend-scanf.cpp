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

#include "hphp/runtime/base/zend-scanf.h"

#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"

///////////////////////////////////////////////////////////////////////////////
/*
  scanf.c --

  This file contains the base code which implements sscanf and by extension
  fscanf. Original code is from TCL8.3.0 and bears the following copyright:

  This software is copyrighted by the Regents of the University of
  California, Sun Microsystems, Inc., Scriptics Corporation,
  and other parties.  The following terms apply to all files associated
  with the software unless explicitly disclaimed in individual files.

  The authors hereby grant permission to use, copy, modify, distribute,
  and license this software and its documentation for any purpose, provided
  that existing copyright notices are retained in all copies and that this
  notice is included verbatim in any distributions. No written agreement,
  license, or royalty fee is required for any of the authorized uses.
  Modifications to this software may be copyrighted by their authors
  and need not follow the licensing terms described here, provided that
  the new terms are clearly indicated on the first page of each file where
  they apply.

  IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
  FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
  ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
  DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

  THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
  IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
  NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.

  GOVERNMENT USE: If you are acquiring this software on behalf of the
  U.S. government, the Government shall have only "Restricted Rights"
  in the software and related documentation as defined in the Federal
  Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
  are acquiring the software on behalf of the Department of Defense, the
  software shall be classified as "Commercial Computer Software" and the
  Government shall have only "Restricted Rights" as defined in Clause
  252.227-7013 (c) (1) of DFARs.  Notwithstanding the foregoing, the
  authors grant the U.S. Government and others acting in its behalf
  permission to use and distribute the software in accordance with the
  terms specified in this license.
*/

#define SCAN_MAX_ARGS   0xFF    // Maximum number of variable which can be
                                // passed to (f|s)scanf. This is an artifical
                                // upper limit to keep resources in check and
                                // minimize the possibility of exploits

#define SCAN_ERROR_INTERNAL           (SCAN_ERROR_WRONG_PARAM_COUNT - 1)

/*
 * Flag values used internally by [f|s]canf.
 */
#define SCAN_NOSKIP     0x1       /* Don't skip blanks. */
#define SCAN_SUPPRESS   0x2       /* Suppress assignment. */
#define SCAN_UNSIGNED   0x4       /* Read an unsigned value. */
#define SCAN_WIDTH      0x8       /* A width value was supplied. */

#define SCAN_SIGNOK     0x10      /* A +/- character is allowed. */
#define SCAN_NODIGITS   0x20      /* No digits have been scanned. */
#define SCAN_NOZERO     0x40      /* No zero digits have been scanned. */
#define SCAN_XOK        0x80      /* An 'x' is allowed. */
#define SCAN_PTOK       0x100     /* Decimal point is allowed. */
#define SCAN_EXPOK      0x200     /* An exponent is allowed. */

#define UCHAR(x) (unsigned char)(x)

///////////////////////////////////////////////////////////////////////////////
/*
 * The following structure contains the information associated with
 * a character set.
 */
  struct Range {
  char start;
  char end;
};

typedef struct CharSet {
  int exclude;    /* 1 if this is an exclusion set. */
  int nchars;
  char *chars;
  int nranges;
  Range *ranges;
} CharSet;

namespace HPHP {
/**
 *----------------------------------------------------------------------
 *
 * BuildCharSet --
 *
 *  This function examines a character set format specification
 *  and builds a CharSet containing the individual characters and
 *  character ranges specified.
 *
 * Results:
 *  Returns the next format position.
 *
 * Side effects:
 *  Initializes the charset.
 *
 *----------------------------------------------------------------------
 */
static const char *BuildCharSet(CharSet *cset, const char *format) {
  const char *ch;
  char start;
  int  nranges;
  const char *end;

  memset(cset, 0, sizeof(CharSet));

  ch = format;
  if (*ch == '^') {
    cset->exclude = 1;
    ch = ++format;
  }
  end = format + 1;  /* verify this - cc */

  /*
   * Find the close bracket so we can overallocate the set.
   */
  if (*ch == ']') {
    ch = end++;
  }
  nranges = 0;
  while (*ch != ']') {
    if (*ch == '-') {
      nranges++;
    }
    ch = end++;
  }

  cset->chars = (char *)malloc(end - format - 1);
  if (nranges > 0) {
    cset->ranges = (::Range*)malloc(sizeof(::Range) * nranges);
  } else {
    cset->ranges = nullptr;
  }

  /*
   * Now build the character set.
   */
  cset->nchars = cset->nranges = 0;
  ch    = format++;
  start = *ch;
  if (*ch == ']' || *ch == '-') {
    cset->chars[cset->nchars++] = *ch;
    ch = format++;
  }
  while (*ch != ']') {
    if (*format == '-') {
      /*
       * This may be the first character of a range, so don't add
       * it yet.
       */
      start = *ch;
    } else if (*ch == '-') {
      /*
       * Check to see if this is the last character in the set, in which
       * case it is not a range and we should add the previous character
       * as well as the dash.
       */
      if (*format == ']') {
        cset->chars[cset->nchars++] = start;
        cset->chars[cset->nchars++] = *ch;
      } else {
        ch = format++;

        /*
         * Check to see if the range is in reverse order.
         */
        if (start < *ch) {
          cset->ranges[cset->nranges].start = start;
          cset->ranges[cset->nranges].end = *ch;
        } else {
          cset->ranges[cset->nranges].start = *ch;
          cset->ranges[cset->nranges].end = start;
        }
        cset->nranges++;
      }
    } else {
      cset->chars[cset->nchars++] = *ch;
    }
    ch = format++;
  }
  return format;
}

/**
 *----------------------------------------------------------------------
 *
 * CharInSet --
 *
 *  Check to see if a character matches the given set.
 *
 * Results:
 *  Returns non-zero if the character matches the given set.
 *
 * Side effects:
 *  None.
 *
 *----------------------------------------------------------------------
 */
static int CharInSet(CharSet *cset, int c) {
  char ch = (char) c;
  int i, match = 0;

  for (i = 0; i < cset->nchars; i++) {
    if (cset->chars[i] == ch) {
      match = 1;
      break;
    }
  }
  if (!match) {
    for (i = 0; i < cset->nranges; i++) {
      if ((cset->ranges[i].start <= ch)
        && (ch <= cset->ranges[i].end)) {
        match = 1;
        break;
      }
    }
  }
  return (cset->exclude ? !match : match);
}

/**
 *----------------------------------------------------------------------
 *
 * ReleaseCharSet --
 *
 *  Free the storage associated with a character set.
 *
 * Results:
 *  None.
 *
 * Side effects:
 *  None.
 *
 *----------------------------------------------------------------------
 */
static void ReleaseCharSet(CharSet *cset) {
  free((char *)cset->chars);
  if (cset->ranges) {
    free((char *)cset->ranges);
  }
}

static inline void scan_set_error_return(int numVars, Variant &return_value) {
  if (numVars) {
    return_value = SCAN_ERROR_EOF; /* EOF marker */
  } else {
    return_value = uninit_null();
  }
}

/**
 *----------------------------------------------------------------------
 *
 * ValidateFormat --
 *
 *  Parse the format string and verify that it is properly formed
 *  and that there are exactly enough variables on the command line.
 *
 * Parameters :
 *     format     The format string.
 *     numVars    The number of variables passed to the scan command.
 *     totalSubs  The number of variables that will be required.
 *
 *----------------------------------------------------------------------
*/
static int ValidateFormat(const char *format, int numVars, int *totalSubs) {
#define STATIC_LIST_SIZE 16
  int gotXpg, gotSequential, value, i, flags;
  const char *end, *ch = nullptr;
  int staticAssign[STATIC_LIST_SIZE];
  int *nassign = staticAssign;
  int objIndex, xpgSize, nspace = STATIC_LIST_SIZE;

  /*
   * Initialize an array that records the number of times a variable
   * is assigned to by the format string.  We use this to detect if
   * a variable is multiply assigned or left unassigned.
   */
  if (numVars > nspace) {
    nassign = (int*)malloc(sizeof(int) * numVars);
    nspace = numVars;
  }
  for (i = 0; i < nspace; i++) {
    nassign[i] = 0;
  }

  xpgSize = objIndex = gotXpg = gotSequential = 0;

  while (*format != '\0') {
    ch = format++;
    flags = 0;

    if (*ch != '%') {
      continue;
    }
    ch = format++;
    if (*ch == '%') {
      continue;
    }
    if (*ch == '*') {
      flags |= SCAN_SUPPRESS;
      ch = format++;
      goto xpgCheckDone;
    }

    if ( isdigit( (int)*ch ) ) {
      /*
       * Check for an XPG3-style %n$ specification.  Note: there
       * must not be a mixture of XPG3 specs and non-XPG3 specs
       * in the same format string.
       */
      char *endptr;
      value = strtoul(format-1, &endptr, 10);
      end = endptr;
      if (*end != '$') {
        goto notXpg;
      }
      format = end+1;
      ch     = format++;
      gotXpg = 1;
      if (gotSequential) {
        goto mixedXPG;
      }
      objIndex = value - 1;
      if ((objIndex < 0) || (numVars && (objIndex >= numVars))) {
        goto badIndex;
      } else if (numVars == 0) {
        /*
         * In the case where no vars are specified, the user can
         * specify %9999$ legally, so we have to consider special
         * rules for growing the assign array.  'value' is
         * guaranteed to be > 0.
         */

        /* set a lower artificial limit on this
         * in the interest of security and resource friendliness
         * 255 arguments should be more than enough. - cc
         */
        if (value > SCAN_MAX_ARGS) {
          goto badIndex;
        }

        xpgSize = (xpgSize > value) ? xpgSize : value;
      }
      goto xpgCheckDone;
    }

notXpg:
    gotSequential = 1;
    if (gotXpg) {
mixedXPG:
      if (nassign != staticAssign) free((char *)nassign);
      throw_invalid_argument
        ("format: cannot mix \"%%\" and \"%%n$\" conversion specifiers");
      return SCAN_ERROR_INVALID_FORMAT;
    }

xpgCheckDone:
    /*
     * Parse any width specifier.
     */
    if (isdigit(UCHAR(*ch))) {
      char *endptr;
      value = strtoul(format-1, &endptr, 10);
      format = endptr;
      flags |= SCAN_WIDTH;
      ch = format++;
    }

    /*
     * Ignore size specifier.
     */
    if ((*ch == 'l') || (*ch == 'L') || (*ch == 'h')) {
      ch = format++;
    }

    if (!(flags & SCAN_SUPPRESS) && numVars && (objIndex >= numVars)) {
      goto badIndex;
    }

    /*
     * Handle the various field types.
     */
    switch (*ch) {
    case 'n':
    case 'd':
    case 'D':
    case 'i':
    case 'o':
    case 'x':
    case 'X':
    case 'u':
    case 'f':
    case 'e':
    case 'E':
    case 'g':
    case 's':
      break;

    case 'c':
      /* we differ here with the TCL implementation in allowing for */
      /* a character width specification, to be more consistent with */
      /* ANSI. since Zend auto allocates space for vars, this is no */
      /* problem - cc                                               */
      /*
        if (flags & SCAN_WIDTH) {
        throw_invalid_argument
        ("format: Field width may not be specified in %c conversion");
        }
        return SCAN_ERROR_INVALID_FORMAT;
      */
      break;

    case '[':
      if (*format == '\0') {
        goto badSet;
      }
      ch = format++;
      if (*ch == '^') {
        if (*format == '\0') {
          goto badSet;
        }
        ch = format++;
      }
      if (*ch == ']') {
        if (*format == '\0') {
          goto badSet;
        }
        ch = format++;
      }
      while (*ch != ']') {
        if (*format == '\0') {
          goto badSet;
        }
        ch = format++;
      }
      break;
    badSet:
      if (nassign != staticAssign) free((char *)nassign);
      throw_invalid_argument("format: Unmatched [ in format string");
      return SCAN_ERROR_INVALID_FORMAT;

    default:
      if (nassign != staticAssign) free((char *)nassign);
      throw_invalid_argument("Bad scan conversion character \"%c\"", *ch);
      return SCAN_ERROR_INVALID_FORMAT;
    }

    if (!(flags & SCAN_SUPPRESS)) {
      if (objIndex >= nspace) {
        /*
         * Expand the nassign buffer.  If we are using XPG specifiers,
         * make sure that we grow to a large enough size.  xpgSize is
         * guaranteed to be at least one larger than objIndex.
         */
        value = nspace;
        if (xpgSize) {
          nspace = xpgSize;
        } else {
          nspace += STATIC_LIST_SIZE;
        }
        if (nassign == staticAssign) {
          nassign = (int*)malloc(nspace * sizeof(int));
          for (i = 0; i < STATIC_LIST_SIZE; ++i) {
            nassign[i] = staticAssign[i];
          }
        } else {
          nassign = (int*)realloc((void *)nassign, nspace * sizeof(int));
        }
        for (i = value; i < nspace; i++) {
          nassign[i] = 0;
        }
      }
      nassign[objIndex]++;
      objIndex++;
    }
  } /* while (*format != '\0') */

  /*
   * Verify that all of the variable were assigned exactly once.
   */
  if (numVars == 0) {
    if (xpgSize) {
      numVars = xpgSize;
    } else {
      numVars = objIndex;
    }
  }
  if (totalSubs) {
    *totalSubs = numVars;
  }
  for (i = 0; i < numVars; i++) {
    if (nassign[i] > 1) {
      if (nassign != staticAssign) free((char *)nassign);
      throw_invalid_argument
        ("format: Variable is assigned by multiple \"%%n$\" specifiers");
      return SCAN_ERROR_INVALID_FORMAT;
    } else if (!xpgSize && (nassign[i] == 0)) {
      /*
       * If the space is empty, and xpgSize is 0 (means XPG wasn't
       * used, and/or numVars != 0), then too many vars were given
       */
      if (nassign != staticAssign) free((char *)nassign);
      throw_invalid_argument
        ("format: Variable is not assigned by any conversion specifiers");
      return SCAN_ERROR_INVALID_FORMAT;
    }
  }

  if (nassign != staticAssign) free((char *)nassign);
  return SCAN_SUCCESS;

badIndex:
  if (nassign != staticAssign) free((char *)nassign);
  if (gotXpg) {
    throw_invalid_argument
      ("format: \"%%n$\" argument index out of range");
  } else {
    throw_invalid_argument
      ("format: Different numbers of variable names and field specifiers");
  }
  return SCAN_ERROR_INVALID_FORMAT;
#undef STATIC_LIST_SIZE
}

/**
 * This is the internal function which does processing on behalf of
 * both sscanf() and fscanf()
 *
 * parameters :
 *    string    literal string to be processed
 *    format    format string
 *    return_value set with the results of the scan
 */
int string_sscanf(const char *string, const char *format, int numVars,
                  Variant &return_value) {
  int  nconversions;
  int  totalVars = -1;
  int  value;
  char *end;
  const char *baseString;
  char op   = 0;
  int  base = 0;
  int  underflow = 0;
  size_t width;
  long (*fn)(const char *, char **, int) = nullptr;
  const char *ch;
  char sch;
  int  flags;
  char buf[64];  /* Temporary buffer to hold scanned number
                  * strings before they are passed to strtoul() */

  /*
   * Check for errors in the format string.
   */
  if (ValidateFormat(format, numVars, &totalVars) != SCAN_SUCCESS) {
    scan_set_error_return(numVars, return_value);
    return SCAN_ERROR_INVALID_FORMAT;
  }

  baseString = string;

  /*
   * Iterate over the format string filling in the result objects until
   * we reach the end of input, the end of the format string, or there
   * is a mismatch.
   */
  nconversions = 0;

  while (*format != '\0') {
    ch    = format++;
    flags = 0;

    /*
     * If we see whitespace in the format, skip whitespace in the string.
     */
    if ( isspace( (int)*ch ) ) {
      sch = *string;
      while ( isspace( (int)sch ) ) {
        if (*string == '\0') {
          goto done;
        }
        string++;
        sch = *string;
      }
      continue;
    }

    if (*ch != '%') {
    literal:
      if (*string == '\0') {
        underflow = 1;
        goto done;
      }
      sch = *string;
      string++;
      if (*ch != sch) {
        goto done;
      }
      continue;
    }

    ch = format++;
    if (*ch == '%') {
      goto literal;
    }

    /*
     * Check for assignment suppression ('*') or an XPG3-style
     * assignment ('%n$').
     */
    if (*ch == '*') {
      flags |= SCAN_SUPPRESS;
      ch = format++;
    } else if ( isdigit(UCHAR(*ch))) {
      value = strtoul(format-1, &end, 10);
      if (*end == '$') {
        format = end+1;
        ch = format++;
      }
    }

    /*
     * Parse any width specifier.
     */
    if ( isdigit(UCHAR(*ch))) {
      char *endptr;
      width = strtoul(format-1, &endptr, 10);
      format = endptr;
      ch = format++;
    } else {
      width = 0;
    }

    /*
     * Ignore size specifier.
     */
    if ((*ch == 'l') || (*ch == 'L') || (*ch == 'h')) {
      ch = format++;
    }

    /*
     * Handle the various field types.
     */
    switch (*ch) {
    case 'n':
      if (!(flags & SCAN_SUPPRESS)) {
        return_value.append((int)(string - baseString));
      }
      nconversions++;
      continue;

    case 'd':
    case 'D':
      op = 'i';
      base = 10;
      fn = (long (*)(const char *, char **, int))strtol;
      break;
    case 'i':
      op = 'i';
      base = 0;
      fn = (long (*)(const char *, char **, int))strtol;
      break;
    case 'o':
      op = 'i';
      base = 8;
      fn = (long (*)(const char *, char **, int))strtol;
      break;
    case 'x':
    case 'X':
      op = 'i';
      base = 16;
      fn = (long (*)(const char *, char **, int))strtol;
      break;
    case 'u':
      op = 'i';
      base = 10;
      flags |= SCAN_UNSIGNED;
      fn = (long (*)(const char *, char **, int))strtoul;
      break;

    case 'f':
    case 'e':
    case 'E':
    case 'g':
      op = 'f';
      break;

    case 's':
      op = 's';
      break;

    case 'c':
      op = 's';
      flags |= SCAN_NOSKIP;
      /*-cc-*/
      if (0 == width) {
        width = 1;
      }
      /*-cc-*/
      break;
    case '[':
      op = '[';
      flags |= SCAN_NOSKIP;
      break;
    } /* switch */

    /*
     * At this point, we will need additional characters from the
     * string to proceed.
     */
    if (*string == '\0') {
      underflow = 1;
      goto done;
    }

    /*
     * Skip any leading whitespace at the beginning of a field unless
     * the format suppresses this behavior.
     */
    if (!(flags & SCAN_NOSKIP)) {
      while (*string != '\0') {
        sch = *string;
        if (! isspace((int)sch) ) {
          break;
        }
        string++;
      }
      if (*string == '\0') {
        underflow = 1;
        goto done;
      }
    }

    /*
     * Perform the requested scanning operation.
     */
    switch (op) {
    case 'c':
    case 's':
      /*
       * Scan a string up to width characters or whitespace.
       */
      if (width == 0) {
        width = (size_t) ~0;
      }
      end = (char*)string;
      while (*end != '\0') {
        sch = *end;
        if ( isspace( (int)sch ) ) {
          break;
        }
        end++;
        if (--width == 0) {
          break;
        }
      }
      if (!(flags & SCAN_SUPPRESS)) {
        return_value.append(String(string, end-string, CopyString));
      }
      string = end;
      break;

    case '[': {
      CharSet cset;

      if (width == 0) {
        width = (size_t) ~0;
      }
      end = (char*)string;

      format = BuildCharSet(&cset, format);
      while (*end != '\0') {
        sch = *end;
        if (!CharInSet(&cset, (int)sch)) {
          break;
        }
        end++;
        if (--width == 0) {
          break;
        }
      }
      ReleaseCharSet(&cset);

      if (string == end) {
        /*
         * Nothing matched the range, stop processing
         */
        goto done;
      }
      if (!(flags & SCAN_SUPPRESS)) {
        return_value.append(String(string, end-string, CopyString));
      }
      string = end;
      break;
    }
    case 'i':
      /*
       * Scan an unsigned or signed integer.
       */
      /*-cc-*/
      buf[0] = '\0';
      /*-cc-*/
      if ((width == 0) || (width > sizeof(buf) - 1)) {
        width = sizeof(buf) - 1;
      }

      flags |= SCAN_SIGNOK | SCAN_NODIGITS | SCAN_NOZERO;
      for (end = buf; width > 0; width--) {
        switch (*string) {
          /*
           * The 0 digit has special meaning at the beginning of
           * a number.  If we are unsure of the base, it
           * indicates that we are in base 8 or base 16 (if it is
           * followed by an 'x').
           */
        case '0':
          /*-cc-*/
          if (base == 16) {
            flags |= SCAN_XOK;
          }
          /*-cc-*/
          if (base == 0) {
            base = 8;
            flags |= SCAN_XOK;
          }
          if (flags & SCAN_NOZERO) {
            flags &= ~(SCAN_SIGNOK | SCAN_NODIGITS | SCAN_NOZERO);
          } else {
            flags &= ~(SCAN_SIGNOK | SCAN_XOK | SCAN_NODIGITS);
          }
          goto addToInt;

        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7':
          if (base == 0) {
            base = 10;
          }
          flags &= ~(SCAN_SIGNOK | SCAN_XOK | SCAN_NODIGITS);
          goto addToInt;

        case '8': case '9':
          if (base == 0) {
            base = 10;
          }
          if (base <= 8) {
            break;
          }
          flags &= ~(SCAN_SIGNOK | SCAN_XOK | SCAN_NODIGITS);
          goto addToInt;

        case 'A': case 'B': case 'C':
        case 'D': case 'E': case 'F':
        case 'a': case 'b': case 'c':
        case 'd': case 'e': case 'f':
          if (base <= 10) {
            break;
          }
          flags &= ~(SCAN_SIGNOK | SCAN_XOK | SCAN_NODIGITS);
          goto addToInt;

        case '+': case '-':
          if (flags & SCAN_SIGNOK) {
            flags &= ~SCAN_SIGNOK;
            goto addToInt;
          }
          break;

        case 'x': case 'X':
          if ((flags & SCAN_XOK) && (end == buf+1)) {
            base = 16;
            flags &= ~SCAN_XOK;
            goto addToInt;
          }
          break;
        }

        /*
         * We got an illegal character so we are done accumulating.
         */
        break;

      addToInt:
        /*
         * Add the character to the temporary buffer.
         */
        *end++ = *string++;
        if (*string == '\0') {
          break;
        }
      }

      /*
       * Check to see if we need to back up because we only got a
       * sign or a trailing x after a 0.
       */
      if (flags & SCAN_NODIGITS) {
        if (*string == '\0') {
          underflow = 1;
        }
        goto done;
      } else if (end[-1] == 'x' || end[-1] == 'X') {
        end--;
        string--;
      }

      /*
       * Scan the value from the temporary buffer.  If we are
       * returning a large unsigned value, we have to convert it back
       * to a string since PHP only supports signed values.
       */
      if (!(flags & SCAN_SUPPRESS)) {
        *end = '\0';
        value = (int) (*fn)(buf, nullptr, base);
        if ((flags & SCAN_UNSIGNED) && (value < 0)) {
          snprintf(buf, sizeof(buf), "%u", value); /* INTL: ISO digit */
          return_value.append(String(buf, CopyString));
        } else {
          return_value.append(value);
        }
      }
      break;

    case 'f':
      /*
       * Scan a floating point number
       */
      buf[0] = '\0';     /* call me pedantic */
      if ((width == 0) || (width > sizeof(buf) - 1)) {
        width = sizeof(buf) - 1;
      }
      flags |= SCAN_SIGNOK | SCAN_NODIGITS | SCAN_PTOK | SCAN_EXPOK;
      for (end = buf; width > 0; width--) {
        switch (*string) {
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        case '8': case '9':
          flags &= ~(SCAN_SIGNOK | SCAN_NODIGITS);
          goto addToFloat;
        case '+':
        case '-':
          if (flags & SCAN_SIGNOK) {
            flags &= ~SCAN_SIGNOK;
            goto addToFloat;
          }
          break;
        case '.':
          if (flags & SCAN_PTOK) {
            flags &= ~(SCAN_SIGNOK | SCAN_PTOK);
            goto addToFloat;
          }
          break;
        case 'e':
        case 'E':
          /*
           * An exponent is not allowed until there has
           * been at least one digit.
           */
          if ((flags & (SCAN_NODIGITS | SCAN_EXPOK)) == SCAN_EXPOK) {
            flags = (flags & ~(SCAN_EXPOK|SCAN_PTOK))
              | SCAN_SIGNOK | SCAN_NODIGITS;
            goto addToFloat;
          }
          break;
        }

        /*
         * We got an illegal character so we are done accumulating.
         */
        break;

      addToFloat:
        /*
         * Add the character to the temporary buffer.
         */
        *end++ = *string++;
        if (*string == '\0') {
          break;
        }
      }

      /*
       * Check to see if we need to back up because we saw a
       * trailing 'e' or sign.
       */
      if (flags & SCAN_NODIGITS) {
        if (flags & SCAN_EXPOK) {
          /*
           * There were no digits at all so scanning has
           * failed and we are done.
           */
          if (*string == '\0') {
            underflow = 1;
          }
          goto done;
        }

        /*
         * We got a bad exponent ('e' and maybe a sign).
         */
        end--;
        string--;
        if (*end != 'e' && *end != 'E') {
          end--;
          string--;
        }
      }

      /*
       * Scan the value from the temporary buffer.
       */
      if (!(flags & SCAN_SUPPRESS)) {
        double dvalue;
        *end = '\0';
        dvalue = strtod(buf, nullptr);
        return_value.append(dvalue);
      }
      break;
    } /* switch (op) */
    nconversions++;
  } /*  while (*format != '\0') */

done:
  if (underflow && (0==nconversions)) {
    scan_set_error_return(numVars, return_value);
    return SCAN_ERROR_EOF;
  } else if (nconversions < totalVars) {
    /* TODO: not all elements converted. we need to prune the list - cc */
  }
  return SCAN_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
}
