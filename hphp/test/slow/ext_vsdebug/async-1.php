<?hh

function skipEvents() {
  $msg = json_decode(getNextVsDebugMessage(), true);
  while ($msg['type'] == 'event') {
    if ($msg['event'] === 'output') {
      $body = $msg['body'];
      printf("OUT: %s", $body['output']);
    }
    $msg = json_decode(getNextVsDebugMessage(), true);
  }
  return $msg;
}

function send($expr) {
  return sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => $expr,
      'context' => 'repl',
      'frameId' => 2]]);
}

function expect_int($seq, $i) {
  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    'body' => dict[
      'type' => 'int',
      'result' => $i,
    ],
    "request_seq" => $seq]);
}

function expect_success($seq) {
  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    "request_seq" => $seq]);
}

<<__EntryPoint>>
function main(): void {
  chdir(__DIR__);
  require('./common.inc');
  $path = __FILE__ . ".test";
  $testProcess = vsDebugLaunch($path, true, varray[]);

  sendVsCommand(darray[
    'command' => 'stackTrace',
    'type' => 'request',
    'arguments' => darray['threadId' => 1],
  ]);
  skipEvents();

  send('await foo_async();') |> expect_int($$, '5');
  send('await bar_async();') |> expect_int($$, '15');

  send('await foo_async() + 5;') |> expect_int($$, '10');
  send('5 + await foo_async();') |> expect_int($$, '10');
  send('await foo_async() + await bar_async();') |> expect_int($$, '20');

  send('return await foo_async() + 5;') |> expect_int($$, '10');
  send('return 5 + await foo_async();') |> expect_int($$, '10');
  send('return await foo_async() + await bar_async();') |> expect_int($$, '20');

  send('concurrent { $x = await foo_async(); $y = await bar_async(); };') |> expect_success($$);
  send('$x + $y') |> expect_int($$, '20');

  resumeTarget();
  vsDebugCleanup($testProcess);
  echo "Ok!\n";
}
