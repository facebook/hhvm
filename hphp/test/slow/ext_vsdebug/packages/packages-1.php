<?hh

<<__EntryPoint>>
function main_packages(): void {
  chdir(__DIR__);
  require('./../common.inc');
  $path = __FILE__ . ".test";
  $testProcess = vsDebugLaunch($path, true, vec[]);

  sendVsCommand(dict[
    'command' => 'stackTrace',
    'type' => 'request',
    'arguments' => dict['threadId' => 1],
  ]);
  skip_events();

  send_eval('(int)package_exists(\'foo\');') |> expect_int($$, "1");
  send_eval('(int)package_exists(\'bar\');') |> expect_int($$, "1");
  send_eval('(int)package_exists(\'nope\');') |> expect_int($$, "0");

  resumeTarget();
  vsDebugCleanup($testProcess);
  echo "Ok!\n";
}
