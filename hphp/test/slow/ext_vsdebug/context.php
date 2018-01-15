<?hh
require(__DIR__ . '/common.inc');

$path = __DIR__ . "/context.php.test";
$GLOBALS["path"] = $path;
$GLOBALS["file"] = "context.php.test";

// Calibration mappings:
$mapping[20] = 20;
$mapping[21] = 21;
$mapping[23] = 24;
$mapping[28] = 28;
$GLOBALS['mapping'] = $mapping;

/*
 * Breakpoint tests - valid breakpoints, resolution, calibration
 */
$setBreakpointsCommand = array(
  "command" => "setBreakpoints",
  "type" => "request",
  "seq" => 3,
  "arguments" => array(
    "source" =>
      array(
        "path" => $path,
        "name" => "context.php.test"
      ),
    "lines" => [20, 21, 23, 28],
    "breakpoints" => [
      array("line" => 20, "condition" => ""),
      array("line" => 21, "condition" => ""),
      array("line" => 23, "condition" => ""),
      array("line" => 28, "condition" => "")
    ]
  ));

$setBreakpointsRepsponse = array(
  "type" => "response",
  "command" => "setBreakpoints",
  "success" => true,
  "request_seq" => 3,
  "body" => array(
    "breakpoints" => array(
      array("id" => 1, "verified" => false),
      array("id" => 2, "verified" => false),
      array("id" => 3, "verified" => false),
      array("id" => 4, "verified" => false)
    )));

$testProcess = vsDebugLaunch(
  __DIR__ . '/context.php.test',
  true,
  [$setBreakpointsCommand],
  [
    // Response event
    $setBreakpointsRepsponse,

    // New BP event for each breakpoint
    bpEvent($path, 20, 1, "new", false),
    bpEvent($path, 21, 2, "new", false),
    bpEvent($path, 23, 3, "new", false),
    bpEvent($path, 28, 4, "new", false),

    // Resolved BP event for each bp
    bpEvent($path, 20, 1, "changed", true),
    bpEvent($path, 21, 2, "changed", true),
    bpEvent($path, 23, 3, "changed", true),
    bpEvent($path, 28, 3, "changed", true)
  ]
);

// Verify we resumed from loader break.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => false
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "continued",
  "body" => array(
      "allThreadsContinued" => true,
      "threadId" => 1
  )));

// Verify we hit breakpoint 1.
verifyBpHit(1, 20);

// Check thread stacks.
sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "seq" => 10,
  "arguments" => array(
    "threadId" => 1
  )));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "stackTrace",
  "request_seq" => 10,
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
sendVsCommand(array(
  "command" => "threads",
  "type" => "request",
  "seq" => 11));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "threads",
  "request_seq" => 11,
  "success" => true,
  "body" => [
    array("name" => "Request 1",
          "id" => 1
    )
  ]
  ));


// Get scopes.
sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "seq" => 12,
  "arguments" => array("frameId" => 1)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "scopes",
  "request_seq" => 12,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 13,
  "arguments" => array("variablesReference" => 3)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 13,
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
verifyBpHit(2, 21);
resumeTarget();

// Verify we hit breakpoint 3.
verifyBpHit(3, 23);
resumeTarget();

// Verify we hit breakpoint 4.
verifyBpHit(4, 28);

sendVsCommand(array(
  "command" => "stackTrace",
  "type" => "request",
  "seq" => 10,
  "arguments" => array(
    "threadId" => 1
  )));
$msg = json_decode(getNextVsDebugMessage(), true);

sendVsCommand(array(
  "command" => "scopes",
  "type" => "request",
  "seq" => 14,
  "arguments" => array("frameId" => 8)));
$msg = json_decode(getNextVsDebugMessage(), true);

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 15,
  "arguments" => array("variablesReference" => 10)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => 15,
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

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 16,
  "arguments" => array("variablesReference" => 17)));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => 16,
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
          "name" => "Static Properties",
          "value" => "class B",
          "namedVariables" => 1,
          "presentationHint" => array(
            "attributes" => ["constant", "readOnly"]
          )
        ),
    ]]
    ));

// Check that we can see the correct properties of $aObj
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 17,
  "arguments" => array("variablesReference" => 19)));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => 17,
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
          "name" => "Static Properties",
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 18,
  "arguments" => array("variablesReference" => 20)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => 18,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 19,
  "arguments" => array("variablesReference" => 22)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => 19,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 20,
  "arguments" => array("variablesReference" => 21)));
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
    "type" => "response",
    "command" => "variables",
    "request_seq" => 20,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 21,
  "arguments" => array("variablesReference" => 15)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => 21,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 22,
  "arguments" =>array("variablesReference" => 15, "count" => 2)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => 22,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 23,
  "arguments" => array("variablesReference" => 16)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => 23,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 24,
  "arguments" => array("variablesReference" => 25)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => 24,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 25,
  "arguments" => array("variablesReference" => 18)));
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively($msg, array(
      "type" => "response",
      "command" => "variables",
      "request_seq" => 25,
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

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 26,
  "arguments" => array("variablesReference" => 26)));

$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "response",
  "command" => "variables",
  "request_seq" => 26,
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
        "name" => "Static Properties",
        "value" => "class B",
        "namedVariables" => 1,
        "presentationHint" => array(
          "attributes" => ["constant", "readOnly"]
        )
      ),
  ]]
  ));

  // Ask for a subset of the array. Give me index 1 only.
  sendVsCommand(array(
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
        "request_seq" => 27,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 28,
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
    "request_seq" => 28,
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
sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 29,
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
    "request_seq" => 29,
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

sendVsCommand(array(
  "command" => "variables",
  "type" => "request",
  "seq" => 30,
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
    "request_seq" => 30,
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

// Verify that the script exited.
$msg = json_decode(getNextVsDebugMessage(), true);
checkObjEqualRecursively($msg, array(
  "type" => "event",
  "event" => "thread",
  "body" => array(
    "threadId" => 1,
    "reason" => "exited"
  )));

// Read anything left it stdout and stderr and echo it.
$stdout = $testProcess[1][1];
$stderr = $testProcess[1][2];
echo stream_get_contents($stdout);
echo stream_get_contents($stderr);
vsDebugCleanup($testProcess[0], $testProcess[1], $testProcess[2]);
