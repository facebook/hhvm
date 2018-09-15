<?hh
require(__DIR__ . '/common.inc');

/*
 * This test verifies the correct sequence of debugger startup messages.
 */

$testProcess = vsDebugLaunch(__FILE__ . ".test", true);

checkForOutput($testProcess, "hello world.", "stdout");

vsDebugCleanup($testProcess);
