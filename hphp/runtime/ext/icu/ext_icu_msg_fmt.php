<?hh

/**
 * MessageFormatter is a concrete class that enables users to produce
 * concatenated, language-neutral messages. The methods supplied in this class
 * are used to build all the messages that are seen by end users.   The
 * MessageFormatter class assembles messages from various fragments (such as
 * text fragments, numbers, and dates) supplied by the program. Because of the
 * MessageFormatter class, the program does not need to know the order of the
 * fragments. The class uses the formatting specifications for the fragments
 * to assemble them into a message that is contained in a single string within
 * a resource bundle. For example, MessageFormatter enables you to print the
 * phrase "Finished printing x out of y files..." in a manner that still
 * allows for flexibility in translation.   Previously, an end user message
 * was created as a sentence and handled as a string. This procedure created
 * problems for localizers because the sentence structure, word order, number
 * format and so on are very different from language to language. The
 * language-neutral way to create messages keeps each part of the message
 * separate and provides keys to the data. Using these keys, the
 * MessageFormatter class can concatenate the parts of the message, localize
 * them, and display a well-formed string to the end user.   MessageFormatter
 * takes a set of objects, formats them, and then inserts the formatted
 * strings into the pattern at the appropriate places. Choice formats can be
 * used in conjunction with MessageFormatter to handle plurals, match numbers,
 * and select from an array of items. Typically, the message format will come
 * from resources and the arguments will be dynamically set at runtime.
 */
<<__NativeData("MessageFormatter")>>
class MessageFormatter {
  /**
   * Constructs a new Message Formatter
   *
   * @param string $locale - The locale to use when formatting arguments
   * @param string $pattern - The pattern string to stick arguments into.
   *   The pattern uses an 'apostrophe-friendly' syntax; it is run through
   *   umsg_autoQuoteApostrophe before being interpreted.
   *
   * @return MessageFormatter - The formatter object
   */
  <<__Native>>
  public function __construct(string $locale,
                              string $pattern): void;

  /**
   * Constructs a new Message Formatter
   *
   * @param string $locale - The locale to use when formatting arguments
   * @param string $pattern - The pattern string to stick arguments into.
   *   The pattern uses an 'apostrophe-friendly' syntax; it is run through
   *   umsg_autoQuoteApostrophe before being interpreted.
   *
   * @return MessageFormatter - The formatter object
   */
  public static function create(string $locale,
                                string $pattern): ?MessageFormatter {
    try {
      return new MessageFormatter($locale, $pattern);
    } catch (Exception $e) {
      return null;
    }
  }

  /**
   * Quick format message
   *
   * @param string $locale - The locale to use for formatting
   *   locale-dependent parts
   * @param string $pattern - The pattern string to insert things into.
   *   The pattern uses an 'apostrophe-friendly' syntax; it is run through
   *   umsg_autoQuoteApostrophe before being interpreted.
   * @param array $args - The array of values to insert into the format
   *   string
   *
   * @return string - The formatted pattern string or FALSE if an error
   *   occurred
   */
  public static function formatMessage(string $locale,
                                       string $pattern,
                                       array $args): mixed {
    $fmt = new MessageFormatter($locale, $pattern);
    return $fmt->format($args);
  }

  /**
   * Format the message
   *
   * @param array $args - Arguments to insert into the format string
   *
   * @return string - The formatted string, or FALSE if an error occurred
   */
  <<__Native>>
  public function format(array $args): mixed;

  /**
   * Get the error code from last operation
   *
   * @return int - The error code, one of UErrorCode values. Initial
   *   value is U_ZERO_ERROR.
   */
  <<__Native>>
  public function getErrorCode(): int;

  /**
   * Get the error text from the last operation
   *
   * @return string - Description of the last error.
   */
  <<__Native>>
  public function getErrorMessage(): string;

  /**
   * Get the locale for which the formatter was created.
   *
   * @return string - The locale name
   */
  <<__Native>>
  public function getLocale(): string;

  /**
   * Get the pattern used by the formatter
   *
   * @return string - The pattern string for this message formatter
   */
  <<__Native>>
  public function getPattern(): string;

  /**
   * Quick parse input string
   *
   * @param string $locale - The locale to use for parsing
   *   locale-dependent parts
   * @param string $pattern - The pattern with which to parse the value.
   * @param string $source - The string to parse, conforming to the
   *   pattern.
   *
   * @return array - An array containing items extracted, or FALSE on
   *   error
   */
  public static function parseMessage(string $locale,
                                      string $pattern,
                                      string $source): mixed {
    $fmt = new MessageFormatter($locale, $pattern);
    return $fmt->parse($source);
  }

  /**
   * Parse input string according to pattern
   *
   * @param string $value - The string to parse
   *
   * @return array - An array containing the items extracted, or FALSE on
   *   error
   */
  <<__Native>>
  public function parse(string $value): mixed;

  /**
   * Set the pattern used by the formatter
   *
   * @param string $pattern - The pattern string to use in this message
   *   formatter. The pattern uses an 'apostrophe-friendly' syntax; it is
   *   run through umsg_autoQuoteApostrophe before being interpreted.
   *
   * @return bool -
   */
  <<__Native>>
  public function setPattern(string $pattern): bool;

}

/**
 * Constructs a new Message Formatter
 *
 * @param string $locale - The locale to use when formatting arguments
 * @param string $pattern - The pattern string to stick arguments into.
 *   The pattern uses an 'apostrophe-friendly' syntax; it is run through
 *   umsg_autoQuoteApostrophe before being interpreted.
 *
 * @return MessageFormatter - The formatter object
 */
function msgfmt_create(string $locale,
                       string $pattern): ?MessageFormatter {
  try {
    return new MessageFormatter($locale, $pattern);
  } catch (Exception $e) {
    return null;
  }
}

/**
 * Quick format message
 *
 * @param string $locale - The locale to use for formatting
 *   locale-dependent parts
 * @param string $pattern - The pattern string to insert things into. The
 *   pattern uses an 'apostrophe-friendly' syntax; it is run through
 *   umsg_autoQuoteApostrophe before being interpreted.
 * @param array $args - The array of values to insert into the format
 *   string
 *
 * @return string - The formatted pattern string or FALSE if an error
 *   occurred
 */
function msgfmt_format_message(string $locale,
                               string $pattern,
                               array $args): string {
  return MessageFormatter::formatMessage($locale, $pattern, $args);
}

/**
 * Format the message
 *
 * @param MessageFormatter $fmt - The message formatter
 * @param array $args - Arguments to insert into the format string
 *
 * @return string - The formatted string, or FALSE if an error occurred
 */
function msgfmt_format(MessageFormatter $fmt,
                       array $args): mixed {
  return $fmt->format($args);
}

/**
 * Get the error code from last operation
 *
 * @param MessageFormatter $fmt - The message formatter
 *
 * @return int - The error code, one of UErrorCode values. Initial value
 *   is U_ZERO_ERROR.
 */
function msgfmt_get_error_code(MessageFormatter $fmt): int {
  return $fmt->getErrorCode();
}

/**
 * Get the error text from the last operation
 *
 * @param MessageFormatter $fmt - The message formatter
 *
 * @return string - Description of the last error.
 */
function msgfmt_get_error_message(MessageFormatter $fmt): string {
  return $fmt->getErrorMessage();
}

/**
 * Get the locale for which the formatter was created.
 *
 * @param MessageFormatter $formatter - The formatter resource
 *
 * @return string - The locale name
 */
function msgfmt_get_locale(MessageFormatter $fmt): string {
  return $fmt->getLocale();
}

/**
 * Get the pattern used by the formatter
 *
 * @param MessageFormatter $fmt - The message formatter
 *
 * @return string - The pattern string for this message formatter
 */
function msgfmt_get_pattern(MessageFormatter $fmt): string {
  return $fmt->getPattern();
}

/**
 * Quick parse input string
 *
 * @param string $locale - The locale to use for parsing locale-dependent
 *   parts
 * @param string $pattern - The pattern with which to parse the value.
 * @param string $value -
 *
 * @return array - An array containing items extracted, or FALSE on error
 */
function msgfmt_parse_message(string $locale,
                              string $pattern,
                              string $value): mixed {
  return MessageFormatter::parseMessage($locale, $pattern, $value);
}

/**
 * Parse input string according to pattern
 *
 * @param MessageFormatter $fmt - The message formatter
 * @param string $value - The string to parse
 *
 * @return array - An array containing the items extracted, or FALSE on
 *   error
 */
function msgfmt_parse(MessageFormatter $fmt,
                      string $value): mixed {
  return $fmt->parse($value);
}

/**
 * Set the pattern used by the formatter
 *
 * @param MessageFormatter $fmt - The message formatter
 * @param string $pattern - The pattern string to use in this message
 *   formatter. The pattern uses an 'apostrophe-friendly' syntax; it is run
 *   through umsg_autoQuoteApostrophe before being interpreted.
 *
 * @return bool -
 */
function msgfmt_set_pattern(MessageFormatter $fmt,
                            string $pattern): bool {
  return $fmt->setPattern($pattern);
}
