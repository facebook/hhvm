/* $Revision: 1.2 $
 **
 ** Do shell-style pattern matching for ?, \, [], and * characters.
 ** Might not be robust in face of malformed patterns; e.g., "foo[a-"
 ** could cause a segmentation violation. It is 8bit clean.
 **
 ** Written by Rich $alz, mirror!rs, Wed Nov 26 19:03:17 EST 1986.
 ** Rich $alz is now <rsalz@osf.org>.
 ** April, 1991: Replaced mutually-recursive calls with in-line code
 ** for the star character.
 **
 ** Special thanks to Lars Mathiesen <thorinn@diku.dk> for the ABORT code.
 ** This can greatly speed up failing wildcard patterns. For example:
 ** pattern: -*-*-*-*-*-*-12-*-*-*-m-*-*-*
 ** text 1: -adobe-courier-bold-o-normal--12-120-75-75-m-70-iso8859-1
 ** text 2: -adobe-courier-bold-o-normal--12-120-75-75-X-70-iso8859-1
 ** Text 1 matches with 51 calls, while text 2 fails with 54 calls. Without
 ** the ABORT code, it takes 22310 calls to fail. Ugh. The following
 ** explanation is from Lars:
 ** The precondition that must be fulfilled is that DoMatch will consume
 ** at least one character in text. This is true if *p is neither '*' nor
 ** '\0'.) The last return has ABORT instead of FALSE to avoid quadratic
 ** behaviour in cases like pattern "*a*b*c*d" with text "abcxxxxx".  With
 ** FALSE, each star-loop has to run to the end of the text; with ABORT
 ** only the last one does.
 **
 ** Once the control of one instance of DoMatch enters the star-loop, that
 ** instance will return either TRUE or ABORT, and any calling instance
 ** will therefore return immediately after (without calling recursively
 ** again). In effect, only one star-loop is ever active. It would be
 ** possible to modify the code to maintain this context explicitly,
 ** eliminating all recursive calls at the cost of some complication and
 ** loss of clarity (and the ABORT stuff seems to be unclear enough by
 ** itself). I think it would be unwise to try to get this into a
 ** released version unless you have a good test data base to try it
 ** out on.
 */

#include "cs_config.h"

#include <ctype.h>
#include "neo_bool.h"
#include "neo_misc.h"

#define ABORT -1


 /* What character marks an inverted character class? */
#define NEGATE_CLASS '^'
 /* Is "*" a common pattern? */
#define OPTIMIZE_JUST_STAR
 /* Do tar(1) matching rules, which ignore a trailing slash? */
#undef MATCH_TAR_PATTERN


/*
 ** Match text and p, return TRUE, FALSE, or ABORT.
 */
  static int 
DoMatch(register const char *text, register const char *p)
{
  register int last;
  register int matched;
  register int reverse;

  for ( ; *p; text++, p++) {
    if (*text == '\0' && *p != '*')
      return ABORT;
    switch (*p) {
      case '\\':
	/* Literal match with following character. */
	p++;
	/* FALLTHROUGH */
      default:
	if (*text != *p)
	  return FALSE;
	continue;
      case '?':
	/* Match anything. */
	continue;
      case '*':
	while (*++p == '*')
	  /* Consecutive stars act just like one. */
	  continue;
	if (*p == '\0')
	  /* Trailing star matches everything. */
	  return TRUE;
	while (*text)
	  if ((matched = DoMatch(text++, p)) != FALSE)
	    return matched;
	return ABORT;
      case '[':
	reverse = p[1] == NEGATE_CLASS ? TRUE : FALSE;
	if (reverse)
	  /* Inverted character class. */
	  p++;
	matched = FALSE;
	if (p[1] == ']' || p[1] == '-')
	  if (*++p == *text)
	    matched = TRUE;
	for (last = *p; *++p && *p != ']'; last = *p)
	  /* This next line requires a good C compiler. */
	  if (*p == '-' && p[1] != ']'
	      ? *text <= *++p && *text >= last : *text == *p)
	    matched = TRUE;
	if (matched == reverse)
	  return FALSE;
	continue;
    }
  }

#ifdef MATCH_TAR_PATTERN
  if (*text == '/')
    return TRUE;
#endif /* MATCH_TAR_ATTERN */
  return *text == '\0';
}


/*
 ** Match text and p, return TRUE, FALSE, or ABORT.
 */
static int
DoMatchCaseInsensitive(register const char *text, register const char *p)
{
  register int last;
  register int matched;
  register int reverse;

  for ( ; *p; text++, p++) {
    if (*text == '\0' && *p != '*')
      return ABORT;
    switch (*p) {
      case '\\':
	/* Literal match with following character. */
	p++;
	/* FALLTHROUGH */
      default:
	if (toupper(*text) != toupper(*p))
	  return FALSE;
	continue;
      case '?':
	/* Match anything. */
	continue;
      case '*':
	while (*++p == '*')
	  /* Consecutive stars act just like one. */
	  continue;
	if (*p == '\0')
	  /* Trailing star matches everything. */
	  return TRUE;
	while (*text)
	  if ((matched = DoMatchCaseInsensitive(text++, p)) != FALSE)
	    return matched;
	return ABORT;
      case '[':
	reverse = p[1] == NEGATE_CLASS ? TRUE : FALSE;
	if (reverse)
	  /* Inverted character class. */
	  p++;
	matched = FALSE;
	if (p[1] == ']' || p[1] == '-')
	  if (toupper(*++p) == toupper(*text))
	    matched = TRUE;
	for (last = toupper(*p); *++p && *p != ']'; last = toupper(*p))
	  /* This next line requires a good C compiler. */
	  if (*p == '-' && p[1] != ']'
	      ? toupper(*text) <= toupper(*++p) && toupper(*text) >= last : toupper(*text) == toupper(*p))
	    matched = TRUE;
	if (matched == reverse)
	  return FALSE;
	continue;
    }
  }

#ifdef MATCH_TAR_PATTERN
  if (*text == '/')
    return TRUE;
#endif /* MATCH_TAR_ATTERN */
  return *text == '\0';
}


/*
 ** User-level routine. Returns TRUE or FALSE.
 */
int
wildmat(const char *text, const char *p)
{
#ifdef OPTIMIZE_JUST_STAR
  if (p[0] == '*' && p[1] == '\0')
    return TRUE;
#endif /* OPTIMIZE_JUST_STAR */
  return DoMatch(text, p) == TRUE;
}

/*
 ** User-level routine. Returns TRUE or FALSE.
 */
int
wildmatcase(const char *text, const char *p)
{
#ifdef OPTIMIZE_JUST_STAR
  if (p[0] == '*' && p[1] == '\0')
    return TRUE;
#endif /* OPTIMIZE_JUST_STAR */
  return DoMatchCaseInsensitive(text, p) == TRUE;
}
