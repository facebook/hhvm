<?hh


<<__EntryPoint>> function main() :mixed{
require(__DIR__ . '/common.inc');
  $path = __FILE__ . '.test';
  $bp1_line = 10;
  $bp2_line = 12;
  $bp1 = dict[
    'line' => $bp1_line,
    'calibratedLine' => $bp1_line,
    'condition' => ''
  ];
  $bp2 = dict[
    'line' => $bp2_line,
    'calibratedLine' => $bp2_line,
    'condition' => ''
  ];

  $breakpoints = vec[
    dict[
      'path' => $path,
      'breakpoints' => vec[$bp1, $bp2],
    ]
  ];

  $testProcess = vsDebugLaunch($path, true, $breakpoints);

  // Skip breakpoint resolution messages.
  skipMessages(count($breakpoints[0]{'breakpoints'}));

  // we should have stopped at the first BP
  verifyBpHit($path, $bp1);

  // in order to inspect variables you have to use the 'stackTrace' command to
  // cause frame ids to be allocated; use the 'scopes' command to cause
  // variables references to be allocated for the constants/globals/locals in
  // scope in a particular frame, and finally the variables command to fetch a
  // variables reference
  $seq = sendVsCommand(dict[
    'command' => 'stackTrace',
    'type' => 'request',
    'arguments' => dict['threadId' => 1],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'stackTrace',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'totalFrames' => 1,
        'stackFrames' => vec[
          dict[
            'id' => 1,
            'source' => dict['path' => $path],
            'line' => $bp1_line,
            'name' => 'main',
          ],
        ],
      ],
    ],
  );

  $seq = sendVsCommand(dict[
    'command' => 'scopes',
    'type' => 'request',
    'arguments' => dict['frameId' => 1]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'scopes',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'scopes' => vec[
          dict[
            'name' => 'Locals',
            'namedVariables' => 1,
            'variablesReference' => 2,
          ],
          dict[
            'name' => 'Superglobals',
            'namedVariables' => 7,
            'variablesReference' => 3,
          ],
          dict[
            'name' => 'Constants',
            'namedVariables' => 2,
            'variablesReference' => 4,
          ],
        ],
      ],
    ],
  );

  // Locals
  $seq = sendVsCommand(dict[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => dict['variablesReference' => 2]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'variables' => vec[
          dict[
            'name' => '$x',
            'type' => 'int',
            'value' => '7',
          ],
        ],
      ],
    ],
  );

  // Superglobals
  $seq = sendVsCommand(dict[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => dict['variablesReference' => 3]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'variables' => vec[
          dict[
            'name' => '$_COOKIE',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          dict[
            'name' => '$_ENV',
            'type' => 'dict',
            // not asserting size of array; ENV may vary
          ],
          dict[
            'name' => '$_FILES',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          dict[
            'name' => '$_GET',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          dict[
            'name' => '$_POST',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          dict[
            'name' => '$_REQUEST',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          dict[
            'name' => '$_SERVER',
            'type' => 'dict',
            // not asserting size of array as it may vary
          ],
        ],
      ],
    ],
  );

  // Constants; we'll have to chase more variablesReferences from here
  $seq = sendVsCommand(dict[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => dict['variablesReference' => 4]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'variables' => vec[
          dict[
            'name' => 'System Defined Constants',
            'variablesReference' => 11,
            // not asserting number of namedVariables, it will change often
          ],
          dict[
            'name' => 'User Defined Constants',
            'variablesReference' => 10,
            'namedVariables' => 2,
          ],
        ],
      ],
    ],
  );

  // User Defined Constants
  $seq = sendVsCommand(dict[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => dict['variablesReference' => 10]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'variables' => vec[
          dict[
            'name' => 'RUNTIME_INIT',
            'type' => 'int',
            'value' => '2',
          ],
          dict[
            'name' => 'SIMPLE',
            'type' => 'int',
            'value' => '1',
          ],
        ],
      ],
    ],
  );

  // change the value of a local via the REPL
  $seq = sendVsCommand(dict[
    'command' => 'evaluate',
    'type' => 'request',
    'arguments' => dict[
      'frameId' => 1,
      'expression' => '$x = 8;',
    ],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'evaluate',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'type' => 'int',
        'result' => '8',
      ],
    ],
  );

  // set a local via the REPL that doesn't exist in the frame
  $seq = sendVsCommand(dict[
    'command' => 'evaluate',
    'type' => 'request',
    'arguments' => dict[
      'frameId' => 1,
      'expression' => '$y = 99;',
    ],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'evaluate',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'type' => 'int',
        'result' => '99',
      ],
    ],
  );

  // Pull locals again and see changes we made via the REPL
  $seq = sendVsCommand(dict[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => dict['variablesReference' => 2]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'variables' => vec[
          dict[
            'name' => '$x',
            'type' => 'int',
            'value' => '8',
          ],
          dict[
            'name' => '$y',
            'type' => 'int',
            'value' => '99',
          ],
        ],
      ],
    ],
  );

  resumeTarget();
  checkForOutput($testProcess, "hello world 1\n", 'stdout');

  // we should have stopped at the second BP
  verifyBpHit($path, $bp2);

  // go through the process of looking at locals at the second BP
  $seq = sendVsCommand(dict[
    'command' => 'stackTrace',
    'type' => 'request',
    'arguments' => dict['threadId' => 1],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'stackTrace',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'totalFrames' => 1,
        'stackFrames' => vec[
          dict[
            'id' => 12,
            'source' => dict['path' => $path],
            'line' => $bp2_line,
            'name' => 'main',
          ],
        ],
      ],
    ],
  );

  $seq = sendVsCommand(dict[
    'command' => 'scopes',
    'type' => 'request',
    'arguments' => dict['frameId' => 12]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'scopes',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'scopes' => vec[
          dict[
            'name' => 'Locals',
            'namedVariables' => 2,
            'variablesReference' => 2,
          ],
          dict[
            'name' => 'Superglobals',
            'namedVariables' => 7,
            'variablesReference' => 3,
          ],
          dict[
            'name' => 'Constants',
            'namedVariables' => 2,
            'variablesReference' => 4,
          ],
        ],
      ],
    ],
  );

  // we see mutations to locals that happened between our 2 BPs, and can still
  // see the nonexistent local we set earlier
  $seq = sendVsCommand(dict[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => dict['variablesReference' => 2]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'variables' => vec[
          dict[
            'name' => '$x',
            'type' => 'int',
            'value' => '9',
          ],
          dict[
            'name' => '$y',
            'type' => 'int',
            'value' => '99',
          ],
        ],
      ],
    ],
  );

  // change the value of our local again, debugged code running after we
  // resume will observe it
  $seq = sendVsCommand(dict[
    'command' => 'evaluate',
    'type' => 'request',
    'arguments' => dict[
      'frameId' => 12,
      'expression' => '$x = 2;',
    ],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    dict[
      'command' => 'evaluate',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => dict[
        'type' => 'int',
        'result' => '2',
      ],
    ],
  );

  resumeTarget();
  checkForOutput($testProcess, "hello world 2\n", 'stdout');

  vsDebugCleanup($testProcess);
  echo "OK\n";
}
