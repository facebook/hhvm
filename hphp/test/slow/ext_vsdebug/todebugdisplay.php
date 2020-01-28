<?hh
require(__DIR__ . '/common.inc');
<<__EntryPoint>> function main(): void {
$path = __FILE__ . ".test";
$breakpoints = varray[
   darray[
     "path" => $path,
     "breakpoints" => varray[
       darray["line" => 14, "calibratedLine" => 14, "condition" => ""],
     ]]
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Check thread stacks.
$seq = sendVsCommand(darray[
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => darray[
    "threadId" => 1
  ]
]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => $seq,
  "success" => true,
  "body" => darray[
      "totalFrames" => 2,
      "stackFrames" => varray[
        darray[
          "source" => darray["path" => $path, "name" => str_replace(".test", "", basename($path))],
          "id" => 1,
          "line" => 14,
          "name" => "innerFunc"
        ],
        darray[
          "source" => darray["path" => $path, "name" => str_replace(".test", "", basename($path))],
          "id" => 2,
          "line" => 17,
          "name" => "{main}"
        ]
      ]
    ]
  ]
);

// Check threads.
$seq = sendVsCommand(darray[
  "command" => "threads",
  "type" => "request"]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, darray[
  "type" => "response",
  "command" => "threads",
  "request_seq" => $seq,
  "success" => true,
  "body" => darray[
    "threads" => varray[darray["name" => "Request 1", "id" => 1]]
  ]
  ]);

// Get scopes.
$seq = sendVsCommand(darray[
  "command" => "scopes",
  "type" => "request",
  "arguments" => darray["frameId" => 1]]);
$msg = json_decode(getNextVsDebugMessage(), true);

checkObjEqualRecursively($msg, darray[
  "type" => "response",
  "command" => "scopes",
  "request_seq" => $seq,
  "success" => true,
  "body" => darray[
    "scopes" => varray[
      darray[
        "namedVariables" => 1,
        "name" => "Locals",
      ],
      darray[
        "namedVariables" => 8,
        "name" => "Superglobals",
      ],
      darray[
        "namedVariables" => 2,
        "name" => "Constants",
      ]
  ]]
  ]);

// Get locals, only $a should be visible right here.
$seq = sendVsCommand(darray[
  "command" => "variables",
  "type" => "request",
  "arguments" => darray["variablesReference" => 3]]);
$msg = json_decode(getNextVsDebugMessage(), true);

if (isset($msg['body']['variables'][0]['variablesReference'])) {
  throw new ErrorException('A class with a __toDebugDisplay method should not return variablesReference');
}

checkObjEqualRecursively($msg, darray[
  "type" => "response",
  "command" => "variables",
  "request_seq" => $seq,
  "success" => true,
  "body" => darray[
    "variables" => varray[
      darray[
        "type" => "A",
        "name" => "\$a",
        "value" => "A(42)",
      ],
  ]]
]);

resumeTarget();

checkForOutput($testProcess, "lol\n", "stdout");
checkForOutput($testProcess, "hello world 2\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}
