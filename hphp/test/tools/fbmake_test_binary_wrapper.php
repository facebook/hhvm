<?hh

/*
 * Small utilities for wrapping tests to put output into fbmake.
 *
 * See hphp/hhvm/fbmake_test_ext_wrapper.php.
 */

// Output is in the format expected by JsonTestRunner.
function say($val) :mixed{
  fwrite(HH\stderr(), json_encode($val, JSON_UNESCAPED_SLASHES) . "\n");
}

function finish($status) :mixed{



  say(dict['op' => 'test_done',
            'test' => ToolsFbmakeTestBinaryWrapperPhp::$current,
            'details' => '',
            'status' => $status]);
  ToolsFbmakeTestBinaryWrapperPhp::$results[] = dict[
    'name'   => ToolsFbmakeTestBinaryWrapperPhp::$current,
    'status' => $status,
  ];
  ToolsFbmakeTestBinaryWrapperPhp::$current = '';
}

function start($test) :mixed{


  ToolsFbmakeTestBinaryWrapperPhp::$current = $test;
  say(dict['op'    => 'start',
            'test'  => ToolsFbmakeTestBinaryWrapperPhp::$current]);
}

function test_is_running() :mixed{
  return \HH\global_get('current') != '';
}

function loop_tests($cmd, $line_func) :mixed{


  $ftest = popen($cmd, 'r');
  if (!$ftest) {
    echo "Couldn't run test script\n";
    exit(1);
  }
  while (!feof($ftest)) {
    $line = fgets($ftest);
    if ($line !== false) {
      $line_func($line);
    }
  }
  if (!fclose($ftest)) {

    if (ToolsFbmakeTestBinaryWrapperPhp::$current !== '') {
      finish('failed');
    }
    start('test-binary');
    finish('failed');
    return;
  }

  say(dict['op'      => 'all_done',
            'results' => ToolsFbmakeTestBinaryWrapperPhp::$results]);
}


abstract final class ToolsFbmakeTestBinaryWrapperPhp {
  public static $results;
  public static $current;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  chdir(__DIR__.'/../../../');
  $argv = \HH\global_get('argv');
  $cmd = sprintf(
    "./hphp/tools/run_test_binary.sh ".
    "'%s' '%s' '%s' '%s' ".
    "2>/dev/null",
    $argv[1],
    $argv[2],
    $argv[3],
    $argv[4],
  );

  // Currently running test, and the results of each test.
  ToolsFbmakeTestBinaryWrapperPhp::$results = vec[];
  ToolsFbmakeTestBinaryWrapperPhp::$current = '';

  loop_tests($cmd, function ($line) {
    $m = null;
    if (preg_match_with_matches('/^(Test[a-zA-Z]*)\.\.\.\.\.\.$/', $line, inout $m)) {
      start($m[1]);
    } else if (preg_match_with_matches('/^Test[a-zA-Z]* (OK|\#\#\#\#\#\>\>\> FAILED)/',
                 $line,
                 inout $m)) {
      finish($m[1] == 'OK' ? 'passed' : 'failed');
    }
  });
}
