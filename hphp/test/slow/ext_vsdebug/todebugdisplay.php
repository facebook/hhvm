<?hh
<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
$path = __FILE__ . ".test";
$breakpoints = vec[
   dict[
     "path" => $path,
     "breakpoints" => vec[
       dict["line" => 14, "calibratedLine" => 14, "condition" => ""],
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
  ]
]);

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
          "line" => 14,
          "name" => "innerFunc"
        ],
        dict[
          "source" => dict["path" => $path, "name" => str_replace(".test", "", basename($path))],
          "id" => 2,
          "line" => 17,
          "name" => "main"
        ]
      ]
    ]
  ]
);

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

// Get scopes.
$seq = sendVsCommand(dict[
  "command" => "scopes",
  "type" => "request",
  "arguments" => dict["frameId" => 1]]);
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

// Get locals, only $a should be visible right here.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 3]]);
$msg = json_decode(getNextVsDebugMessage(), true);

if (isset($msg['body']['variables'][0]['variablesReference'])) {
  throw new ErrorException('A class with a __toDebugDisplay method should not return variablesReference');
}

checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "variables",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "variables" => vec[
      dict[
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
