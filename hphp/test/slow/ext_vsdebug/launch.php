<?hh
require(__DIR__ . '/common.inc');
$testProcess = vsDebugLaunch(__DIR__ . '/launch.php.test');

/*
 * Launch script test: just launch a script with no breakpoints set.
 * This test verifies that the launch/initialize/configuration done sequence
 * works, and the script starts running.
 */
echo "OK.\n";

// Read anything left it stdout and stderr and echo it.
$stdout = $testProcess[1][1];
$stderr = $testProcess[1][2];
echo stream_get_contents($stdout);
echo stream_get_contents($stderr);
vsDebugCleanup($testProcess[0], $testProcess[1], $testProcess[2]);
