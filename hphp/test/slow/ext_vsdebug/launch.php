<?hh

/*
 * This test verifies the correct sequence of debugger startup messages.
 */
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$testProcess = vsDebugLaunch(__FILE__ . ".test", true);

checkForOutput($testProcess, "hello world.", "stdout");

vsDebugCleanup($testProcess);
}
