<?hh

/* Constants not found in ICU library */
const int IDNA_CONTAINS_ACE_PREFIX = 8;
const int IDNA_CONTAINS_MINUS = 4;
const int IDNA_CONTAINS_NON_LDH = 3;
const int IDNA_ICONV_ERROR = 9;
const int IDNA_INVALID_LENGTH = 5;
const int IDNA_MALLOC_ERROR = 201;
const int IDNA_NO_ACE_PREFIX = 6;
const int IDNA_PUNYCODE_ERROR = 2;
const int IDNA_ROUNDTRIP_VERIFY_ERROR = 7;
const int IDNA_STRINGPREP_ERROR = 1;

/**
 * Useful to handle errors occurred in static methods when there's no object
 * to get error code from.
 *
 * @return int - Error code returned by the last API function call.
 */
<<__Native>>
function intl_get_error_code(): int;

/**
 * Get error message from last internationalization function called.
 *
 * @return string - Description of an error occurred in the last API function
 * call.
 */
<<__Native>>
function intl_get_error_message(): string;

/**
 * Return ICU error code name.
 *
 * @param int $errorCode - ICU error code.
 * @return string - The returned string will be the same as the name of the
 *                  error code constant.
 */
<<__Native>>
function intl_error_name(int $errorCode): string;

/**
 * @param int $errorCode - Value returned by intl_get_error_code()
 * @return bool - Whether the code represents an error of not.
 */
<<__Native>>
function intl_is_failure(int $errorCode): bool;

/**
 * This function converts Unicode domain name to IDNA ASCII-compatible format.
 *
 * @param string $domain - Domain to convert. In PHP 5 must be UTF-8 encoded.
 * @param int $options - Conversion options - combination of IDNA_* constants
 *                       (except IDNA_ERROR_* constants).
 * @param int $varaitn - Either INTL_IDNA_VARIANT_2003 for IDNA 2003 or
 *                       INTL_IDNA_VARIANT_UTS46 for UTS #46.
 *
 * @return mixed - Domain name encoded in ASCII-compatible form
 *                 or FALSE on failure.
 */
<<__Native>>
function idn_to_ascii(string $domain,
                      int $options = 0,
                      int $variant = 0): mixed;

/**
 * @param string $domain
 * @param int $options - Conversion options - combination of IDNA_* constants
 *                       (except IDNA_ERROR_* constants).
 * @param int $variant - Either INTL_IDNA_VARIANT_2003 for IDNA 2003 or
 *                       INTL_IDNA_VARIANT_UTS46 for UTS #46.
 *
 * @return mixed - Domain name in unicode or FALSE on failure.
 */
<<__Native>>
function idn_to_utf8(string $domain,
                     int $options = 0,
                     int $variant = 0): mixed;

/**
 * @alias idn_to_utf8
 */
function idn_to_unicode(string $domain,
                        int $options = 0,
                        int $variant = 0): mixed {
  return idn_to_utf8($domain, $options, $variant);
}
