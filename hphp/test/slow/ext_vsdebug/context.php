<?hh

<<__EntryPoint>> function main() :mixed{
require(__DIR__ . '/common.inc');

$path = __FILE__ . ".test";
$breakpoints = vec[
   dict[
     "path" => $path,
     "breakpoints" => vec[
       dict["line" => 20, "calibratedLine" => 20, "condition" => ""],
       dict["line" => 21, "calibratedLine" => 21, "condition" => ""],
       dict["line" => 23, "calibratedLine" => 24, "condition" => ""],
       dict["line" => 28, "calibratedLine" => 28, "condition" => ""],
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
          "line" => 20,
          "name" => "innerFunc"
        ],
        dict[
          "source" => dict["path" => $path, "name" => str_replace(".test", "", basename($path))],
          "id" => 2,
          "line" => 31,
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
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "variables",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "variables" => vec[
      dict[
        "type" => "int",
        "name" => "\$a",
        "value" => "1",
      ],
  ]]
  ]);

if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

resumeTarget();

// Verify we hit breakpoint 2.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[1]);
resumeTarget();

// Verify we hit breakpoint 3.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[2]);
resumeTarget();

// Verify we hit breakpoint 4.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[3]);

$seq = sendVsCommand(dict[
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => dict[
    "threadId" => 1
  ]]);
$msg = json_decode(getNextVsDebugMessage(), true);

$seq = sendVsCommand(dict[
  "command" => "scopes",
  "type" => "request",
  "arguments" => dict["frameId" => 8]]);
$msg = json_decode(getNextVsDebugMessage(), true);

$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 3]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "int",
          "name" => "\$a",
          "value" => "1",
        ],
        dict[
          "type" => "string",
          "name" => "\$b",
          "value" => "Hello world",
        ],
        dict[
          "type" => "B",
          "name" => "\$bObj",
          "value" => "B",
        ],
        dict[
          "type" => "vec",
          "name" => "\$c",
          "value" => "vec[3]",
          "indexedVariables" => 3,
        ],
        dict[
          "type" => "vec",
          "name" => "\$d",
          "value" => "vec[2]",
          "indexedVariables" => 2,
        ],
        dict[
          "type" => "vec",
          "name" => "\$e",
          "value" => "vec[2]",
          "indexedVariables" => 2,
        ],
    ]]
    ]);

if (count($msg{"body"}{"variables"}) != 6) {
  throw new UnexpectedValueException("Unexpected variable count");
}

$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 14]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "A",
          "name" => "aObj",
          "value" => "A",
          "namedVariables" => 2,
          "variablesReference" => 16,
          "presentationHint" => dict[
            "visibility" => "public"
          ]
        ],
        dict[
          "type" => "int",
          "name" => "b",
          "value" => "2",
          "presentationHint" => dict[
            "visibility" => "protected"
          ]
        ],
        dict[
          "type" => "int",
          "name" => "c",
          "value" => "3",
          "presentationHint" => dict[
            "visibility" => "public"
          ]
        ],

        // The private props should contain the base class's copy of $a, only.
        dict[
          "variablesReference" => 17,
          "name" => "Private props",
          "value" => "class A",
          "namedVariables" => 1,
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],

        // Two constants should be visible on A, HELLOA and HELLOB, and one
        // on class B.
        dict[
          "variablesReference" => 18,
          "name" => "Class Constants",
          "value" => "class B",
          "namedVariables" => 3,
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],

        dict[
          "variablesReference" => 19,
          "name" => "Static Props",
          "value" => "class B",
          "namedVariables" => 1,
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],
    ]]
    ]);

// Check that we can see the correct properties of $aObj
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 16]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "int",
          "name" => "a",
          "value" => "0",
          "presentationHint" => dict[
            "visibility" => "private"
          ]
        ],
        dict[
          "type" => "int",
          "name" => "b",
          "value" => "1",
          "presentationHint" => dict[
            "visibility" => "protected"
          ]
        ],
        dict[
          "name" => "Class Constants",
          "value" => "class A",
          "namedVariables" => 2,
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],

        dict[
          "name" => "Static Props",
          "value" => "class A",
          "namedVariables" => 1,
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],
      ]
    ]]);

if (count($msg{"body"}{"variables"}) != 4) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Correct private props of $bObj from base class A
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 17]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "int",
          "name" => "a",
          "value" => "0",
          "presentationHint" => dict[
            "visibility" => "private"
          ]
        ]
      ]
    ]]);

if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Correct statics, $bObj should see statics inherited from class A
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 19]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "int",
          "name" => "B::\$S",
          "value" => "100",
          "presentationHint" => dict[
            "visibility" => "public"
          ]
        ]
      ]
    ]]);

if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Correct class constants. Should inclide consts from A and B, sorted by name.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 18]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "string",
          "name" => "A::HELLOA",
          "value" => "hello0",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],
        dict[
          "type" => "string",
          "name" => "A::HELLOB",
          "value" => "hello0",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],
        dict[
          "type" => "string",
          "name" => "B::HELLOB",
          "value" => "hello1",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ]
      ]
    ]]);

if (count($msg{"body"}{"variables"}) != 3) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Check $c, a regular array of ints.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 12]]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, dict[
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => dict[
        "variables" => vec[
          dict[
            "type" => "int",
            "name" => "0",
            "value" => "1"
          ],
          dict[
            "type" => "int",
            "name" => "1",
            "value" => "2"
          ],
          dict[
            "type" => "int",
            "name" => "2",
            "value" => "3"
          ]
        ]
      ]]);

if (count($msg{"body"}{"variables"}) != 3) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Ask for a subset of the array.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" =>dict["variablesReference" => 12, "count" => 2]]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, dict[
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => dict[
        "variables" => vec[
          dict[
            "type" => "int",
            "name" => "0",
            "value" => "1"
          ],
          dict[
            "type" => "int",
            "name" => "1",
            "value" => "2"
          ]
        ]
      ]]);

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Check $d, a array of arrays.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 13]]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, dict[
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => dict[
        "variables" => vec[
          dict[
            "type" => "int",
            "name" => "0",
            "value" => "1"
          ],
          dict[
            "type" => "vec",
            "name" => "1",
            "value" => "vec[2]",
            "variablesReference" => 22
          ]
        ]
      ]]);

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// check $d[1], sub array of ints.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 22]]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, dict[
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => dict[
        "variables" => vec[
          dict[
            "type" => "int",
            "name" => "0",
            "value" => "2"
          ],
          dict[
            "type" => "int",
            "name" => "1",
            "value" => "3"
          ]
        ]
      ]]);

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Check $e, array of objects
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 15]]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, dict[
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => dict[
        "variables" => vec[
          dict[
            "type" => "B",
            "name" => "0",
            "value" => "B",
            "variablesReference" => 14
          ],
          dict[
            "type" => "B",
            "name" => "1",
            "value" => "B",
            "variablesReference" => 14
          ]
        ]
      ]]);

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" => dict["variablesReference" => 14]]);

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
  "type" => "response",
  "command" => "variables",
  "request_seq" => $seq,
  "success" => true,
  "body" => dict[
    "variables" => vec[
      dict[
        "type" => "A",
        "name" => "aObj",
        "value" => "A",
        "namedVariables" => 2,
        "variablesReference" => 16,
        "presentationHint" => dict[
          "visibility" => "public"
        ]
      ],
      dict[
        "type" => "int",
        "name" => "b",
        "value" => "2",
        "presentationHint" => dict[
          "visibility" => "protected"
        ]
      ],
      dict[
        "type" => "int",
        "name" => "c",
        "value" => "3",
        "presentationHint" => dict[
          "visibility" => "public"
        ]
      ],

      // The private props should contain the base class's copy of $a, only.
      dict[
        "variablesReference" => 23,
        "name" => "Private props",
        "value" => "class A",
        "namedVariables" => 1,
        "presentationHint" => dict[
          "attributes" => vec["constant", "readOnly"]
        ]
      ],

      // Two constants should be visible on A, HELLOA and HELLOB, and one
      // on class B.
      dict[
        "variablesReference" => 24,
        "name" => "Class Constants",
        "value" => "class B",
        "namedVariables" => 3,
        "presentationHint" => dict[
          "attributes" => vec["constant", "readOnly"]
        ]
      ],

      dict[
        "variablesReference" => 25,
        "name" => "Static Props",
        "value" => "class B",
        "namedVariables" => 1,
        "presentationHint" => dict[
          "attributes" => vec["constant", "readOnly"]
        ]
      ],
  ]]
  ]);

  // Ask for a subset of the array. Give me index 1 only.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "seq" => 27,
  "arguments" => dict[
    "variablesReference" => 12,
    "start" => 1,
    "count" => 1
  ]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
          dict[
          "type" => "int",
          "name" => "1",
          "value" => "2"
        ]
      ]
    ]]);

if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Subset of class constants.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" =>
  dict[
    "variablesReference" => 24,
    "start" => 1,
    "count" => 2
  ]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "string",
          "name" => "A::HELLOB",
          "value" => "hello0",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],
        dict[
          "type" => "string",
          "name" => "B::HELLOB",
          "value" => "hello1",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ]
      ]
    ]]);

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Invalid subsets.
$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" =>
  dict[
    "variablesReference" => 24,
    "start" => 100,
    "count" => 1
  ]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "string",
          "name" => "A::HELLOA",
          "value" => "hello0",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],
      ]
    ]]);
if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

$seq = sendVsCommand(dict[
  "command" => "variables",
  "type" => "request",
  "arguments" =>
  dict[
    "variablesReference" => 24,
    "start" => 1,
    "count" => 100
  ]]);
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, dict[
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => dict[
      "variables" => vec[
        dict[
          "type" => "string",
          "name" => "A::HELLOB",
          "value" => "hello0",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ],
        dict[
          "type" => "string",
          "name" => "B::HELLOB",
          "value" => "hello1",
          "presentationHint" => dict[
            "attributes" => vec["constant", "readOnly"]
          ]
        ]
      ]
    ]]);
if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

resumeTarget();

checkForOutput($testProcess, "hello world 1\n", "stdout");
checkForOutput($testProcess, "hello world 2\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
}
