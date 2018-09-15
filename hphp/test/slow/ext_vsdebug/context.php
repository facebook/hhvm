<?hh
require(__DIR__ . '/common.inc');

$path = __FILE__ . ".test";
$breakpoints = [
   array(
     "path" => $path,
     "breakpoints" => [
       array("line" => 20, "calibratedLine" => 20, "condition" => ""),
       array("line" => 21, "calibratedLine" => 21, "condition" => ""),
       array("line" => 23, "calibratedLine" => 24, "condition" => ""),
       array("line" => 28, "calibratedLine" => 28, "condition" => ""),
     ])
   ];

$testProcess = vsDebugLaunch(__FILE__ . ".test", true, $breakpoints);

// Verify we hit breakpoint 1.
verifyBpHit($breakpoints[0]{'path'}, $breakpoints[0]{'breakpoints'}[0]);

// Check thread stacks.
$seq = sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => array(
    "threadId" => 1
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => $seq,
  "success" => true,
  "body" => array(
      "totalFrames" => 2,
      "stackFrames" => [
        array(
          "source" => array("path" => $path, "name" => $path),
          "id" => 1,
          "line" => 20,
          "name" => "innerFunc"
        ),
        array(
          "source" => array("path" => $path, "name" => $path),
          "id" => 2,
          "line" => 31,
          "name" => "{main}"
        )
      ]
    )
  ));

// Check threads.
$seq = sendVsCommand(array(
  "command" => "threads",
  "type" => "request"));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "threads",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "threads" => [array("name" => "Request 1", "id" => 1)]
  ]
  ));


// Get scopes.
$seq = sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "arguments" => array("frameId" => 1)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "scopes",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "scopes" => [
      array(
        "namedVariables" => 1,
        "name" => "Locals",
      ),
      array(
        "namedVariables" => 9,
        "name" => "Superglobals",
      ),
      array(
        "namedVariables" => 2,
        "name" => "Constants",
      )
  ]]
  ));

// Get locals, only $a should be visible right here.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 3)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "type" => "int",
        "name" => "\$a",
        "value" => "1",
      ),
  ]]
  ));

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

$seq = sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "arguments" => array(
    "threadId" => 1
  )));
$msg = json_decode(getNextVsDebugMessage(), true);

$seq = sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "arguments" => array("frameId" => 8)));
$msg = json_decode(getNextVsDebugMessage(), true);

$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 10)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "int",
          "name" => "\$a",
          "value" => "1",
        ),
        array(
          "type" => "string",
          "name" => "\$b",
          "value" => "Hello world",
        ),
        array(
          "type" => "B",
          "name" => "\$bObj",
          "value" => "B",
        ),
        array(
          "type" => "array",
          "name" => "\$c",
          "value" => "array[3]",
          "indexedVariables" => 3,
        ),
        array(
          "type" => "array",
          "name" => "\$d",
          "value" => "array[2]",
          "indexedVariables" => 2,
        ),
        array(
          "type" => "array",
          "name" => "\$e",
          "value" => "array[2]",
          "indexedVariables" => 2,
        ),
    ]]
    ));

if (count($msg{"body"}{"variables"}) != 6) {
  throw new UnexpectedValueException("Unexpected variable count");
}

$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 17)));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "A",
          "name" => "\$aObj",
          "value" => "A",
          "namedVariables" => 2,
          "variablesReference" => 19,
          "presentationHint" => array(
            "visibility" => "public"
          )
        ),
        array(
          "type" => "int",
          "name" => "\$b",
          "value" => "2",
          "presentationHint" => array(
            "visibility" => "protected"
          )
        ),
        array(
          "type" => "int",
          "name" => "\$c",
          "value" => "3",
          "presentationHint" => array(
            "visibility" => "public"
          )
        ),

        // The private props should contain the base class's copy of $a, only.
        array(
          "variablesReference" => 20,
          "name" => "Private props",
          "value" => "class A",
          "namedVariables" => 1,
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),

        // Two constants should be visible on A, HELLOA and HELLOB, and one
        // on class B.
        array(
          "variablesReference" => 21,
          "name" => "Class Constants",
          "value" => "class B",
          "namedVariables" => 3,
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),

        array(
          "variablesReference" => 22,
          "name" => "Static Props",
          "value" => "class B",
          "namedVariables" => 1,
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
    ]]
    ));

// Check that we can see the correct properties of $aObj
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 19)));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "int",
          "name" => "\$a",
          "value" => "0",
          "presentationHint" => array(
            "visibility" => "private"
          )
        ),
        array(
          "type" => "int",
          "name" => "\$b",
          "value" => "1",
          "presentationHint" => array(
            "visibility" => "protected"
          )
        ),
        array(
          "name" => "Class Constants",
          "value" => "class A",
          "namedVariables" => 2,
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),

        array(
          "name" => "Static Props",
          "value" => "class A",
          "namedVariables" => 1,
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
      ]
    ]));

if (count($msg{"body"}{"variables"}) != 4) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Correct private props of $bObj from base class A
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 20)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "int",
          "name" => "\$a",
          "value" => "0",
          "presentationHint" => array(
            "visibility" => "private"
          )
        )
      ]
    ]));

if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Correct statics, $bObj should see statics inherited from class A
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 22)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "int",
          "name" => "B::\$S",
          "value" => "100",
          "presentationHint" => array(
            "visibility" => "public"
          )
        )
      ]
    ]));

if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Correct class constants. Should inclide consts from A and B, sorted by name.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 21)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "string",
          "name" => "A::HELLOA",
          "value" => "hello0",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
        array(
          "type" => "string",
          "name" => "A::HELLOB",
          "value" => "hello0",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
        array(
          "type" => "string",
          "name" => "B::HELLOB",
          "value" => "hello1",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        )
      ]
    ]));

if (count($msg{"body"}{"variables"}) != 3) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Check $c, a regular array of ints.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 15)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => [
        "variables" => [
          array(
            "type" => "int",
            "name" => "0",
            "value" => "1"
          ),
          array(
            "type" => "int",
            "name" => "1",
            "value" => "2"
          ),
          array(
            "type" => "int",
            "name" => "2",
            "value" => "3"
          )
        ]
      ]));

if (count($msg{"body"}{"variables"}) != 3) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Ask for a subset of the array.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" =>array("variablesReference" => 15, "count" => 2)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => [
        "variables" => [
          array(
            "type" => "int",
            "name" => "0",
            "value" => "1"
          ),
          array(
            "type" => "int",
            "name" => "1",
            "value" => "2"
          )
        ]
      ]));

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Check $d, a array of arrays.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 16)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => [
        "variables" => [
          array(
            "type" => "int",
            "name" => "0",
            "value" => "1"
          ),
          array(
            "type" => "array",
            "name" => "1",
            "value" => "array[2]",
            "variablesReference" => 25
          )
        ]
      ]));

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// check $d[1], sub array of ints.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 25)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => [
        "variables" => [
          array(
            "type" => "int",
            "name" => "0",
            "value" => "2"
          ),
          array(
            "type" => "int",
            "name" => "1",
            "value" => "3"
          )
        ]
      ]));

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Check $e, array of objects
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 18)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => $seq,
      "success" => true,
      "body" => [
        "variables" => [
          array(
            "type" => "B",
            "name" => "0",
            "value" => "B",
            "variablesReference" => 26
          ),
          array(
            "type" => "B",
            "name" => "1",
            "value" => "B",
            "variablesReference" => 27
          )
        ]
      ]));

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" => array("variablesReference" => 26)));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => $seq,
  "success" => true,
  "body" => [
    "variables" => [
      array(
        "type" => "A",
        "name" => "\$aObj",
        "value" => "A",
        "namedVariables" => 2,
        "variablesReference" => 28,
        "presentationHint" => array(
          "visibility" => "public"
        )
      ),
      array(
        "type" => "int",
        "name" => "\$b",
        "value" => "2",
        "presentationHint" => array(
          "visibility" => "protected"
        )
      ),
      array(
        "type" => "int",
        "name" => "\$c",
        "value" => "3",
        "presentationHint" => array(
          "visibility" => "public"
        )
      ),

      // The private props should contain the base class's copy of $a, only.
      array(
        "variablesReference" => 29,
        "name" => "Private props",
        "value" => "class A",
        "namedVariables" => 1,
        "presentationHint" => array(
          "attributes" => ["constant", "readOnly"]
        )
      ),

      // Two constants should be visible on A, HELLOA and HELLOB, and one
      // on class B.
      array(
        "variablesReference" => 30,
        "name" => "Class Constants",
        "value" => "class B",
        "namedVariables" => 3,
        "presentationHint" => array(
          "attributes" => ["constant", "readOnly"]
        )
      ),

      array(
        "variablesReference" => 31,
        "name" => "Static Props",
        "value" => "class B",
        "namedVariables" => 1,
        "presentationHint" => array(
          "attributes" => ["constant", "readOnly"]
        )
      ),
  ]]
  ));

  // Ask for a subset of the array. Give me index 1 only.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 27,
  "arguments" => array(
    "variablesReference" => 15,
    "start" => 1,
    "count" => 1
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
          array(
          "type" => "int",
          "name" => "1",
          "value" => "2"
        )
      ]
    ]));

if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Subset of class constants.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" =>
  array(
    "variablesReference" => 21,
    "start" => 1,
    "count" => 2
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "string",
          "name" => "A::HELLOB",
          "value" => "hello0",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
        array(
          "type" => "string",
          "name" => "B::HELLOB",
          "value" => "hello1",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        )
      ]
    ]));

if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

// Invalid subsets.
$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" =>
  array(
    "variablesReference" => 21,
    "start" => 100,
    "count" => 1
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "string",
          "name" => "A::HELLOA",
          "value" => "hello0",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
      ]
    ]));
if (count($msg{"body"}{"variables"}) != 1) {
  throw new UnexpectedValueException("Unexpected variable count");
}

$seq = sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "arguments" =>
  array(
    "variablesReference" => 21,
    "start" => 1,
    "count" => 100
  )));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => $seq,
    "success" => true,
    "body" => [
      "variables" => [
        array(
          "type" => "string",
          "name" => "A::HELLOB",
          "value" => "hello0",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
        array(
          "type" => "string",
          "name" => "B::HELLOB",
          "value" => "hello1",
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        )
      ]
    ]));
if (count($msg{"body"}{"variables"}) != 2) {
  throw new UnexpectedValueException("Unexpected variable count");
}

resumeTarget();

checkForOutput($testProcess, "hello world 1\n", "stdout");
checkForOutput($testProcess, "hello world 2\n", "stdout");
vsDebugCleanup($testProcess);

echo "OK!\n";
