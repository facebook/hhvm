<?hh

<<__EntryPoint>> function main(): void {
  chdir(__DIR__);
  require('./common.inc');
  $path = __FILE__ . ".test";
  $testProcess = vsDebugLaunch($path, true, varray[]);

  send_eval('$a = 76;') |> expect_int($$, '76');

  echo "---- NEW LAMBDA -----\n";
  send_eval('(() ==> $a)();') |> expect_int($$, '76');
  send_eval('$l1 = () ==> $a;') |> expect_success($$);
  send_eval('$l1();') |> expect_int($$, '76');

  echo "---- OLD LAMBDA -----\n";
  send_eval('(function () use ($a) { return $a; })()') |> expect_int($$, '76');
  send_eval('$l2 = function () use ($a) { return $a; };') |> expect_success($$);

  // This one requires frame id to be null, so leave it old style
  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '$l2();',
      'context' => 'repl']]);

  $msg = skip_events();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    'body' => dict[
      'type' => 'int',
      'result' => '76',
    ],
    "request_seq" => $seq]);

  resumeTarget();
  vsDebugCleanup($testProcess);

  echo "----- OK! -----\n";
}
