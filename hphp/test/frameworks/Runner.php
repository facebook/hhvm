<?hh
require_once __DIR__."/Framework.php";
require_once __DIR__.'/Options.php';
require_once __DIR__.'/PHPUnitPatterns.php';
require_once __DIR__.'/Statuses.php';
require_once __DIR__.'/Colors.php';
require_once __DIR__.'/utils.php';
require_once __DIR__.'/ProxyInformation.php';

class Runner {
  // Name could be the name of the single test file for a given framework,
  // or the actual framework name if we are running in serial, for example
  public string $name;

  private string $test_information = "";
  private string $error_information = "";
  private string $fatal_information = "";
  private string $diff_information = "";
  private string $stat_information = "";

  private array<resource> $pipes = null;
  private resource $process = null;
  private string $actual_test_command = "";

  public function __construct(public Framework $framework, string $p = "") {
    $this->name = $p === "" ? $this->framework->getName() : $p;
  }

  public function run(): int {
    chdir($this->framework->getTestPath());
    $ret_val = 0;
    $line = "";
    $post_test = false;
    $pretest_data = true;
    if ($this->initialize()) {
      while (!(feof($this->pipes[1]))) {
        $line = $this->getLine();
        if ($line === null) {
          break;
        }
        if ($this->isBlankLine($line)) {
          continue;
        }
        // e.g. PHPUnit 3.7.28 by Sebastian Bergmann.
        // Even if there are three lines of prologue, this will keep
        // continuing before we call analyzeTest
        if ($this->isPrologue($line)) {
          $pretest_data = false;
          continue;
        }
        // e.g. \nWarning: HipHop currently does not support circular
        // reference collection
        // e.g. No headers testing
        // e.g. Please install runkit and enable runkit.internal_override!
        if ($pretest_data) {
          if ($this->checkForWarnings($line)) {
            $this->error_information .= "PRETEST WARNING FOR ".
              $this->name.PHP_EOL.$line.PHP_EOL.
              $this->getTestRunStr($this->name, "RUN TEST FILE: ").PHP_EOL;
          }
          continue;
        }
        if ($this->isStop($line)) {
          // If we have finished the tests, then we are just printing any error
          // info and getting the final stats
          $this->printPostTestInfo();
          break;
        }
        if (!$pretest_data) {
          // We have gotten through the prologue and any blank lines
          // and we should be at tests now.
          $tn_matches = array();
          if (preg_match($this->framework->getTestNamePattern(), $line,
                         $tn_matches) === 1) {
            // If analyzeTest returns false, then we have most likely
            // hit a fatal. So we bail the run.
            if(!$this->analyzeTest($tn_matches[0])) {
              break;
            }
          } else if ($this->checkForWarnings($line)) {
            // We have a warning after the tests have supposedly started
            // but we really don't have a test to examine.
            // e.g.
            // PHPUnit 3.7.28 by Sebastian Bergmann.
            // The Xdebug extension is not loaded. No code coverage will be gen
            // \nNotice: Use of undefined constant DRIZZLE_CON_NONE
            $line = remove_string_from_text($line, __DIR__, "");
            $this->error_information .= PHP_EOL.$line.PHP_EOL.
              $this->getTestRunStr($this->name,"RUN TEST FILE: ").
              PHP_EOL.PHP_EOL;
            continue;
          } else if ($this->checkForFatals($line)) {
            // We have a fatal after the tests have supposedly started
            // but we really don't have a test to examine.
            // e.g.
            // PHPUnit 3.7.28 by Sebastian Bergmann.
            // The Xdebug extension is not loaded. No code coverage will be gen
            // \nFatal error: Call to undefined function mysqli_report
            $line = remove_string_from_text($line, __DIR__, "");
            $this->fatal_information .= PHP_EOL.$this->name.
              PHP_EOL.$line.PHP_EOL.PHP_EOL.
              $this->getTestRunStr($this->name, "RUN TEST FILE: ").
              PHP_EOL.PHP_EOL;
            break;
          }
        }
      }
      $ret_val = $this->finalize();
      $this->outputData();
    } else {
      error_and_exit("Could not open process to run test ".$this->name.
                     " for framework ".$this->framework->getName(),
                     Options::$csv_only);
    }
    chdir(__DIR__);
    return $ret_val;
  }

  private function analyzeTest(string $test): bool {
    verbose("Analyzing test: ".$test.PHP_EOL, Options::$verbose);
    // If we hit a fatal or something, we will stop the overall test running
    // for this particular test sequence
    $continue_testing = true;
    // We have the test. Now just get the incoming data unitl we find some
    // sort of status data
    do {
      $status = $this->getLine();
      if ($status !== null) {
        // No user specific information in status. Replace with empty string
        $status = remove_string_from_text($status, __DIR__, "");
      }
      if ($status === null) {
        $status = Statuses::UNKNOWN;
        $this->fatal_information .= $test.PHP_EOL.$status.PHP_EOL.
          $this->getTestRunStr($test, "RUN TEST FILE: ").PHP_EOL.PHP_EOL;
        $this->stat_information = $this->name.PHP_EOL.$status.PHP_EOL;
        $continue_testing = false;
        break;
      } else if ($status === Statuses::TIMEOUT) {
        $this->fatal_information .= $test.PHP_EOL.$status.PHP_EOL.
          $this->getTestRunStr($test, "RUN TEST FILE: ").PHP_EOL.PHP_EOL;
        $this->stat_information = $this->name.PHP_EOL.$status.PHP_EOL;
        $continue_testing = false;
        break;
      } else if ($this->checkForFatals($status)) {
        $this->fatal_information .= $test.PHP_EOL.$status.PHP_EOL.PHP_EOL.
          $this->getTestRunStr($test, "RUN TEST FILE: ").PHP_EOL.PHP_EOL;
        $status = Statuses::FATAL;
        $this->stat_information = $this->name.PHP_EOL.$status.PHP_EOL;
        $continue_testing = false;
        break;
      } else if ($this->checkForWarnings($status)) {
        // Warnings are special. We may get one or more warnings, but then
        // a real test status will come afterwards.
        $this->error_information .= $test.PHP_EOL.$status.PHP_EOL.PHP_EOL.
          $this->getTestRunStr($test, "RUN TEST FILE: ").PHP_EOL.PHP_EOL;
        continue;
      }
    } while (!feof($this->pipes[1]) &&
           preg_match(PHPUnitPatterns::$status_code_pattern,
                      $status) === 0);
    // Test names should have all characters before and including __DIR__
    // removed, so that specific user info is not added
    $test = rtrim($test, PHP_EOL);
    $test = remove_string_from_text($test, __DIR__, null);
    $this->test_information .= $test.PHP_EOL;
    $this->processStatus($status, $test);

    return $continue_testing;
  }

  private function processStatus(string $status, string $test): void {
    // May have this if we reached the end of the file or if something
    // wasn't printed out in optimized mode that may have been printed
    // out in debug mode
    if ($status === "" || $status === null) {
      $status = Statuses::UNKNOWN;
    } else if ($status !== Statuses::UNKNOWN && $status !== Statuses::TIMEOUT &&
               $status !== Statuses::FATAL) {
      // Otherwise we have, Fail, Error, Incomplete, Skip, Pass (.)
      // First Char In case we had "F 252 / 364 (69 %)"
      $status = $status[0];
    }

    $this->test_information .= $status.PHP_EOL;

    if ($this->framework->getCurrentTestStatuses() !== null &&
        $this->framework->getCurrentTestStatuses()->containsKey($test)) {
      if ($status === $this->framework->getCurrentTestStatuses()[$test]) {
        // FIX: posix_isatty(STDOUT) was always returning false, even
        // though can print in color. Check this out later.
        verbose(Colors::GREEN.Statuses::PASS.Colors::NONE, !Options::$csv_only);
      } else {
        // Red if we go from pass to something else
        if ($this->framework->getCurrentTestStatuses()[$test] === '.') {
          verbose(Colors::RED.Statuses::FAIL.Colors::NONE, !Options::$csv_only);
        // Green if we go from something else to pass
        } else if ($status === '.') {
          verbose(Colors::GREEN.Statuses::FAIL.Colors::NONE,
                  !Options::$csv_only);
        // Blue if we go from something "faily" to something "faily"
        // e.g., E to I or F
        } else {
          verbose(Colors::BLUE.Statuses::FAIL.Colors::NONE,
                  !Options::$csv_only);
        }
        verbose(PHP_EOL."Different status in ".$this->framework->getName().
                " for test ".$test." was ".
                $this->framework->getCurrentTestStatuses()[$test].
                " and now is ".$status.PHP_EOL, !Options::$csv_only);
        $this->diff_information .= "----------------------".PHP_EOL.
          $test.PHP_EOL.PHP_EOL.
          $this->getTestRunStr($test, "RUN TEST FILE: ").PHP_EOL.PHP_EOL.
          "EXPECTED: ".$this->framework->getCurrentTestStatuses()[$test].
          PHP_EOL.">>>>>>>".PHP_EOL.
          "ACTUAL: ".$status.PHP_EOL.PHP_EOL;
      }
    } else {
      // This is either the first time we run the unit tests, and all pass
      // because we are establishing a baseline. OR we have run the tests
      // before, but we are having an issue getting to the actual tests
      // (e.g., yii is one test suite that has behaved this way).
      if ($this->framework->getCurrentTestStatuses() !== null) {
        verbose(Colors::LIGHTBLUE.Statuses::FAIL.Colors::NONE,
                !Options::$csv_only);
        verbose(PHP_EOL."Different status in ".$this->framework->getName().
                " for test ".$test.PHP_EOL,!Options::$csv_only);
        $this->diff_information .= "----------------------".PHP_EOL.
          "Maybe haven't see this test before: ".$test.PHP_EOL.PHP_EOL.
          $this->getTestRunStr($test, "RUN TEST FILE: ").PHP_EOL.PHP_EOL;
      } else {
        verbose(Colors::GRAY.Statuses::PASS.Colors::NONE, !Options::$csv_only);
      }
    }
  }

  private function getLine(): ?string {
    if (feof($this->pipes[1])) {
      return null;
    }
    if (!$this->checkReadStream()) {
      return Statuses::TIMEOUT;
    }
    $line = stream_get_line($this->pipes[1], 4096, PHP_EOL);
    // No more data
    if ($line === false || $line === null || strlen($line) === 4096) {
      return null;
    }
    $line = remove_color_codes($line);
    return $line;
  }

  // Post test information are error/failure information and the final passing
  // stats for the test
  private function printPostTestInfo(): void {
    $prev_line = null;
    $final_stats = null;
    $matches = array();
    $post_stat_fatal = false;

    // Throw out any initial blank lines
    do {
      $line = $this->getLine();
    } while ($line === "" && $line !== null);

    // Now that we have our first non-blank line, print out the test information
    // until we have our final stats
    while ($line !== null) {
      // Don't print out any of the PHPUnit Patterns to the errors file.
      // Just print out pertinent error information.
      //
      // There was 1 failure:  <---- Don't print
      // <blank line>
      // 1) Assetic\Test\Asset\HttpAssetTest::testGetLastModified <---- print
      if (preg_match(PHPUnitPatterns::$tests_ok_skipped_inc_pattern,
                     $line) === 1 ||
          preg_match(PHPUnitPatterns::$num_errors_failures_pattern,
                     $line) === 1 ||
          preg_match(PHPUnitPatterns::$failures_header_pattern,
                     $line) === 1 ||
          preg_match(PHPUnitPatterns::$num_skips_inc_pattern,
                     $line) === 1) {
        do {
          // throw out any blank lines after these pattern
          $line = $this->getLine();
        } while ($line === "" && $line !== null);
        continue;
      }

      // If we hit what we think is the final stats based on the pattern of the
      // line, make sure this is the case. The final stats will generally be
      // the last line before we hit null returned from line retrieval. The
      // only cases where this would not be true is if, for some rare reason,
      // stat information is part of the information provided for a
      // given test error -- or -- we have hit a fatal at the very end of
      // running PHPUnit. For that fatal case, we handle that a bit differently.
      if (preg_match(PHPUnitPatterns::$tests_ok_pattern, $line) === 1 ||
          preg_match(PHPUnitPatterns::$tests_failure_pattern, $line) === 1 ||
          preg_match(PHPUnitPatterns::$no_tests_executed_pattern,
                     $line) === 1) {
        $prev_line = $line;
        $line = $this->getLine();
        if ($line === null) {
          $final_stats = $prev_line;
          break;
        } else if ($line === "") {
          // FIX ME: The above $line === null check is all I should need, but
          // but getLine() is not cooperating. Not sure if getLine() problem or
          // a PHPUnit output thing, but even when I am at the final stat line
          // pattern, sometimes it takes me two getLine() calls to hit
          // $line === null because I first get $line === "".
          // So...save the current position. Read ahead. If null, we are done.
          // Otherwise, print $prev_line, go back to where we were and the
          // current blank line now stored in $line, will be printed down
          // below
          $curPos = ftell($this->pipes[1]);
          if ($this->getLine() === null) {
            $final_stats = $prev_line;
            break;
          } else {
            $this->error_information .= $prev_line.PHP_EOL;
            fseek($this->pipes[1], $curPos);
          }
        } else if ($this->checkForFatals($line) ||
                   $this->checkForWarnings($line)) {
        // Sometimes when PHPUnit is done printing its post test info, hhvm
        // fatals. This is not good, but it currently happens nonetheless. Here
        // is an example:
        //
        // FAILURES!
        // Tests: 3, Assertions: 9, Failures: 2. <--- EXPECTED LAST LINE (STATS)
        // Core dumped: Segmentation fault  <--- But, we can get this and below
        // /home/joelm/bin/hhvm: line 1: 28417 Segmentation fault
          $final_stats = $prev_line;
          $post_stat_fatal = true;
          break;
        } else {
          $this->error_information .= $prev_line.PHP_EOL;
        }
      }

      $this->error_information .= $line.PHP_EOL;
      if (preg_match($this->framework->getTestNamePattern(), $line,
                     $matches) === 1) {
        $print_blanks = true;
        $this->error_information .= PHP_EOL.
                                    $this->getTestRunStr($matches[0],
                                                         "RUN TEST FILE: ").
                                    PHP_EOL.PHP_EOL;
      }
      $line = $this->getLine();
    }

    if ($post_stat_fatal) {
      $this->fatal_information .= "POST-TEST FATAL/WARNING FOR ".
        $this->name.PHP_EOL.PHP_EOL.
        $this->getTestRunStr($this->name, "RUN TEST FILE: ").
        PHP_EOL.PHP_EOL;
      while ($line !== null) {
        $this->fatal_information .= $line.PHP_EOL;
        $line = $this->getLine();
      }
      // Add a newline to the fatal file if we had a post-test fatal for better
      // visual
      $this->fatal_information .= PHP_EOL;
    }

    // If we have no final stats, assume some sort of fatal for this test.
    // If we have "No tests executed", assume a skip
    // Otherwise, print the final stats.
    $this->stat_information = $this->name.PHP_EOL;
    if ($final_stats === null) {
      $this->stat_information .= Statuses::FATAL.PHP_EOL;
    } else if (preg_match(PHPUnitPatterns::$no_tests_executed_pattern,
                          $final_stats) === 1) {
      $this->stat_information .= Statuses::SKIP.PHP_EOL;
    } else {
      $this->stat_information .= $final_stats.PHP_EOL;
    }
  }

  private function isStop(string $line) {
    return preg_match(PHPUnitPatterns::$stop_parsing_pattern, $line) === 1;
  }

  private function isPrologue(string $line) {
    return preg_match(PHPUnitPatterns::$header_pattern, $line) === 1 ||
        preg_match(PHPUnitPatterns::$config_file_pattern, $line) === 1 ||
        preg_match(PHPUnitPatterns::$xdebug_pattern, $line) === 1;
  }

  private function isBlankLine(string $line): bool {
    return $line === "" || $line === PHP_EOL;
  }

  private function initialize(): bool {
    $this->actual_test_command = $this->framework->getTestCommand($this->name);
    verbose("Command: ".$this->actual_test_command."\n", Options::$verbose);

    $descriptorspec = array(
      0 => array("pipe", "r"),
      1 => array("pipe", "w"),
      2 => array("pipe", "w"),
    );

    $env = $_ENV;
    // Use the proxies in case the test needs access to the outside world
    $env = array_merge($env, ProxyInformation::$proxies->toArray());
    if ($this->framework->getEnvVars() !== null) {
      $env = array_merge($env, $this->framework->getEnvVars()->toArray());
    }
    $this->process = proc_open($this->actual_test_command, $descriptorspec,
                               $this->pipes, $this->framework->getTestPath(),
                               $env);
    return is_resource($this->process);
  }

  private function finalize(): int {
    fclose($this->pipes[0]);
    fclose($this->pipes[1]);
    fclose($this->pipes[2]);

    return proc_close($this->process) === -1 ? -1 : 0;
  }

  private function checkReadStream(): bool {
    $r = array($this->pipes[1]);
    $w = null;
    $e = null;
    $s = stream_select($r, $w, $e, Options::$timeout);
    // If stream_select returns 0, then there is no more data or we have
    // timed out. If it returns false, then something else bad happened.
    return !($s === 0 || $s === false);
  }

  private function outputData(): void {
    file_put_contents($this->framework->getOutFile(), $this->test_information,
                      FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getErrorsFile(),
                      $this->error_information, FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getDiffFile(), $this->diff_information,
                      FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getStatsFile(), $this->stat_information,
                      FILE_APPEND | LOCK_EX);
    file_put_contents($this->framework->getFatalsFile(),
                      $this->fatal_information, FILE_APPEND | LOCK_EX);

  }

  private function checkForFatals(string $line): bool {
    return preg_match(PHPUnitPatterns::$hhvm_fatal_pattern, $line) === 1;
  }

  private function checkForWarnings(string $line): bool {
    return preg_match(PHPUnitPatterns::$hhvm_warning_pattern, $line) === 1 ||
      preg_match(PHPUnitPatterns::$phpunit_exception_with_hhvm_warning,
                 $line) === 1;
  }

  private function getTestRunStr(string $test, string $prologue = "",
                                 string $epilogue = ""): string {
    $test_run = $prologue.
      " cd ".$this->framework->getTestPath()." && ";
    // If the test that is coming in to this function is an individual test,
    // as opposed to a file, then we can use the --filter option to make the
    // run string have even more specificity.
    if (preg_match($this->framework->getTestNamePattern(), $test)) {
      // If we are running this framework with individual test mode
      // (e.g., --by-test), then --filter already exists. We also don't want to
      // add --filter to .phpt style tests (e.g. Pear).
      if (strpos($this->actual_test_command, "--filter") === false &&
          strpos($test, ".phpt") === false) {
        // The string after the last space in actual_test_command is
        // the file that is run in phpunit. Remove the file and replace
        // with --filter <individual test>. This will also get rid of any
        // 2>&1 that may exist as well, which we do not want.
        //
        // e.g.,
        // hhvm -v Eval.Jit=true phpunit --debug 'ConverterTest.php'
        // to
        // hhvm -v Eval.Jit=true phpunit --debug 'ConverterTest::MyTest'
        $t = rtrim(str_replace("2>&1", "", $this->actual_test_command));
        $lastspace = strrpos($t, ' ');
        $t = substr($this->actual_test_command, 0, $lastspace);
        // For --filter, the namespaces need to be separated by \\
        $test = str_replace("\\", "\\\\", $test);
        $t .= " --filter '".$test."'";
        $test_run .= $t;
      } else if (!$this->framework->isParallel()) {
      // If a framework is not being run in parallel (e.g., it is being run like
      // normal phpunit for the entire framework), then the actual_test_command
      // would not contain the individual test by default. It is being run like
      // this, for example, from the test root directory:
      //
      // hhvm phpunit
      //
      // Pear is a current example of this behavior.
        $test_run .= rtrim(str_replace("2>&1", "", $this->actual_test_command));
        // Re-add __DIR__ if not there so we have a full test path to run
        $test_run .= strpos($test, __DIR__) !== 0
          ? " ".__DIR__."/".$test
          : " ".$test;
      } else {
        $test_run .= rtrim(str_replace("2>&1", "", $this->actual_test_command));
      }
    } else {
    // $test is not a XXX::YYY style test, but is instead a file that is already
    // part of the actual_test_comand
      $test_run .= rtrim(str_replace("2>&1", "", $this->actual_test_command));
    }
    return $test_run.$epilogue;
  }
}
