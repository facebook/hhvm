<?hh
class PHPUnitPatterns {
  // Matches:
  //   PrettyExceptionsTest::testReturnsDiagnostics
  //   Assetic\Test\::testMethods with data set #1 ('getRoot')'
  //   Composer\Test\::testMe with data set "parses dates w/ -"
  // The "with data set" can either have a # or " after it and then any char
  // before a resulting " or (
  // Four \\\\ needed to match one \
  // stackoverflow.com/questions/4025482/cant-escape-the-backslash-with-regex
  const string TEST_NAME_PATTERN =
  "/[_a-zA-Z0-9\\\\]*::[_a-zA-Z0-9]*( with data set (\".*?\"|#[0-9]+))?/";

  // Matches:
  //    E
  //    .
  //    .  252 / 364 ( 69%)
  //    .\nWarning
  // That last example happened in Magento
  const string STATUS_CHAR_GROUP = '[\.SFEIR]';
  const string STATUS_CODE_PATTERN = '/^'.self::STATUS_CHAR_GROUP.'$|^S+$|^'.self::STATUS_CHAR_GROUP.'(HipHop)|^'.self::STATUS_CHAR_GROUP.'[ \t]*[0-9]* \/ [0-9]* \([ 0-9]*%\)/';

  // Don't want to parse any more test names after the Time line in the
  // results. Any test names after that line are probably detailed error
  // information.
  const string STOP_PARSING_PATTERN =
 "/^Time: \d+(\.\d+)? (second[s]?|ms|minute[s]?|hour[s]?), Memory: \d+(\.\d+)/";

  const string TESTS_OK_PATTERN = "/^OK \(\d+ test[s]?, \d+ assertion[s]?\)/";
  const string TESTS_FAILURE_PATTERN = "/^Tests: \d+, Assertions: \d+.*[.]/";

  const string HEADER_PATTERN =
                "/^PHPUnit \d+.[0-9a-zA-Z\-\.]*( by Sebastian Bergmann.)?/";

  const string CONFIG_FILE_PATTERN = "/^Configuration read from/";

  const string XDEBUG_PATTERN = "/^The Xdebug extension is not loaded./";

  // Paris and Idiorm have tests with ending digits (e.g. Test53.php)
  const string TEST_FILE_PATTERN =
                "/.*(\.phpt|Test[\d]*\.php|test[\d]*\.php)$/";

  const string TESTS_OK_SKIPPED_INC_PATTERN =
               "/^OK, but incomplete, skipped, or risky tests!/";
  const string NUM_ERRORS_FAILURES_PATTERN =
               "/^There (was|were) \d+ (failure|error)[s]?\:/";
  const string NUM_SKIPS_INC_PATTERN =
               "/^There (was|were) \d+ (skipped|incomplete) test[s]?\:/";
  const string FAILURES_HEADER_PATTERN = "/^FAILURES!/";
  const string NO_TESTS_EXECUTED_PATTERN = "/^No tests executed!/";

  const string WARNING_PATTERN = "/^(Warning|Notice)/";
  const string FATAL_PATTERN =
    "/(^Fatal)|(^hhvm:)|(^Core dumped: Segmentation fault)/";

  const string PHPUNIT_EXCEPTION_WITH_WARNING =
    "/^PHPUnit_Framework_Exception: (Warning|Notice)/";
  const string TEST_METHOD_NAME_PATTERN = "/public function test|\@test/";
}
