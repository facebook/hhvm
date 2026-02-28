<?hh

<<__EntryPoint>>
function main_async_1(): void {
  chdir(__DIR__);
  require('./common.inc');
  $path = __FILE__ . ".test";
  $testProcess = vsDebugLaunch($path, true, vec[]);

  sendVsCommand(dict[
    'command' => 'stackTrace',
    'type' => 'request',
    'arguments' => dict['threadId' => 1],
  ]);
  skip_events();

  send_eval('await foo_async();') |> expect_int($$, '5');
  send_eval('await bar_async();') |> expect_int($$, '15');

  send_eval('await foo_async() + 5;') |> expect_int($$, '10');
  send_eval('5 + await foo_async();') |> expect_int($$, '10');
  send_eval('await foo_async() + await bar_async();') |> expect_int($$, '20');

  send_eval('return await foo_async() + 5;') |> expect_int($$, '10');
  send_eval('return 5 + await foo_async();') |> expect_int($$, '10');
  send_eval('return await foo_async() + await bar_async();') |> expect_int($$, '20');

  send_eval('concurrent { $x = await foo_async(); $y = await bar_async(); };') |> expect_success($$);
  send_eval('$x + $y') |> expect_int($$, '20');

  resumeTarget();
  vsDebugCleanup($testProcess);
  echo "Ok!\n";
}
