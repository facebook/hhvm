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
  static string $test_name_pattern =
  "/[_a-zA-Z0-9\\\\]*::[_a-zA-Z0-9]*( with data set (\".*?\"|#[0-9]+))?/";

  // Matches:
  //    E
  //    .
  //    .  252 / 364 ( 69%)
  //    .\nWarning
  // That last example happened in Magento
  static string $status_code_pattern =
  "/^[\.SFEI]$|^S+$|^[\.SFEI](HipHop)|^[\.SFEI][ \t]*[0-9]* \/ [0-9]* \([ 0-9]*%\)/";

  // Don't want to parse any more test names after the Time line in the
  // results. Any test names after that line are probably detailed error
  // information.
  static string $stop_parsing_pattern =
 "/^Time: \d+(\.\d+)? (second[s]?|ms|minute[s]?|hour[s]?), Memory: \d+(\.\d+)/";

  static string $tests_ok_pattern = "/^OK \(\d+ test[s]?, \d+ assertion[s]?\)/";
  static string $tests_failure_pattern = "/^Tests: \d+, Assertions: \d+.*[.]/";

  static string $header_pattern =
                "/^PHPUnit \d+.[0-9a-zA-Z\-\.]*( by Sebastian Bergmann.)?/";

  static string $config_file_pattern = "/^Configuration read from/";

  static string $xdebug_pattern = "/^The Xdebug extension is not loaded./";

  // Paris and Idiorm have tests with ending digits (e.g. Test53.php)
  static string $test_file_pattern =
                "/.*(\.phpt|Test[\d]*\.php|test[\d]*\.php)$/";

  static string $tests_ok_skipped_inc_pattern =
               "/^OK, but incomplete, skipped, or risky tests!/";
  static string $num_errors_failures_pattern =
               "/^There (was|were) \d+ (failure|error)[s]?\:/";
  static string $num_skips_inc_pattern =
               "/^There (was|were) \d+ (skipped|incomplete) test[s]?\:/";
  static string $failures_header_pattern = "/^FAILURES!/";
  static string $no_tests_executed_pattern = "/^No tests executed!/";

  static string $hhvm_warning_pattern =
                                     "/^(HipHop|HHVM|hhvm) (Warning|Notice)/";
  static string $hhvm_fatal_pattern =
  "/(^(HipHop|HHVM|hhvm) Fatal)|(^hhvm:)|(^Core dumped: Segmentation fault)/";

  static string $phpunit_exception_with_hhvm_warning =
    "/^PHPUnit_Framework_Exception: (HipHop|HHVM|hhvm) (Warning|Notice)/";
  static string $test_method_name_pattern = "/public function test|\@test/";
}
