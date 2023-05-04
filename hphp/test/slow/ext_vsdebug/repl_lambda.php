<?hh

function skipEvents() {
  $msg = json_decode(getNextVsDebugMessage(), true);
  while ($msg['type'] == 'event') {
    if ($msg['event'] === 'output') {
      $body = $msg['body'];
      if ($body['category'] === 'stdout') {
        printf("STDOUT: %s", $body['output']);
      }
    }
    $msg = json_decode(getNextVsDebugMessage(), true);

  }

  return $msg;
}
<<__EntryPoint>> function main(): void {
  chdir(__DIR__);
  require('./common.inc');
  $path = __FILE__ . ".test";
  $testProcess = vsDebugLaunch($path, true, varray[]);

  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '$a = 76;',
      'context' => 'repl']]);

  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    'body' => dict[
      'type' => 'int',
      'result' => '76',
    ],
    "request_seq" => $seq]);

  echo "---- NEW LAMBDA -----\n";

  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '(() ==> $a)();',
      'context' => 'repl']]);

  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    'body' => dict[
      'type' => 'int',
      'result' => '76',
    ],
    "request_seq" => $seq]);

  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '$l1 = () ==> $a;',
      'context' => 'repl']]);

  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    "request_seq" => $seq]);

  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '$l1();',
      'context' => 'repl']]);

  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    'body' => dict[
      'type' => 'int',
      'result' => '76',
    ],
    "request_seq" => $seq]);

  echo "---- OLD LAMBDA -----\n";

  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '(function () use ($a) { return $a; })()',
      'context' => 'repl']]);

  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    'body' => dict[
      'type' => 'int',
      'result' => '76',
    ],
    "request_seq" => $seq]);

  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '$l2 = function () use ($a) { return $a; };',
      'context' => 'repl']]);

  $msg = skipEvents();
  checkObjEqualRecursively($msg, darray[
    "type" => "response",
    "command" => "evaluate",
    "success" => true,
    "request_seq" => $seq]);

  $seq = sendVsCommand(darray[
    "command" => "evaluate",
    "type" => "request",
    "arguments" =>
    darray[
      'expression' => '$l2();',
      'context' => 'repl']]);

  $msg = skipEvents();
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
