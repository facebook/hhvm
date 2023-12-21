<?hh

<<__EntryPoint>> function main() :mixed{
require(__DIR__ . '/common.inc');

$path = __FILE__ . ".test";
$breakpoints = vec[
   dict[
     "path" => $path,
     "breakpoints" => vec[
       dict["line" => 5, "calibratedLine" => 5, "condition" => ""],
     ]]
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);

// Skip breakpoint resolution messages.
skipMessages(count($breakpoints[0]{'breakpoints'}));

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Check thread stacks.
$seq = sendVsCommand(dict[
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => dict[
    "threadId" => 1
  ]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
      "totalFrames" => 2,
      "stackFrames" => vec[
        dict[
          "source" => dict["path" => $path, "name" => str_replace(".test", "", basename($path))],
          "id" => 1,
          "line" => 5,
          "name" => "innerFunc"
        ],
        dict[
          "source" => dict["path" => $path, "name" => str_replace(".test", "", basename($path))],
          "id" => 2,
          "line" => 9,
          "name" => "main"
        ]
      ]
    ]
  ]);

// Check threads.
$seq = sendVsCommand(dict[
  "command" => "threads",
  "type" => "request"]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "threads",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "threads" => vec[dict["name" => "Request 1", "id" => 1]]
  ]
  ]);


// Get scope of the parent frame.
$seq = sendVsCommand(dict[
  "command" => "scopes",
  "type" => "request",
  "arguments" => dict["frameId" => 2]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "scopes",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "scopes" => vec[
      dict[
        "namedVariables" => 1,
        "name" => "Locals",
      ],
      dict[
        "namedVariables" => 7,
        "name" => "Superglobals",
      ],
      dict[
        "namedVariables" => 2,
        "name" => "Constants",
      ]
  ]]
]);

// Evaluate in the current context.
$seq = sendVsCommand(dict[
  "command" => "evaluate",
  "type" => "request",
  "arguments" => dict['expression' => '$x;', 'context' => 'repl']
]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "evaluate",
  "success" => true,
  "body" => dict["type" => "int", "result" => "1"],
  "request_seq" => $seq
  ]);

resumeTarget();

checkForOutput($testProcess, "3", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}
