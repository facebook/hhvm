<?hh


<<__EntryPoint>> function main() {
require(__DIR__ . '/common.inc');
  $path = __FILE__ . '.test';
  $bp1_line = 10;
  $bp2_line = 12;
  $bp1 = darray[
    'line' => $bp1_line,
    'calibratedLine' => $bp1_line,
    'condition' => ''
  ];
  $bp2 = darray[
    'line' => $bp2_line,
    'calibratedLine' => $bp2_line,
    'condition' => ''
  ];

  $breakpoints = varray[
    darray[
      'path' => $path,
      'breakpoints' => varray[$bp1, $bp2],
    ]
  ];

  $testProcess = vsDebugLaunch($path, true, $breakpoints);

  // we should have stopped at the first BP
  verifyBpHit($path, $bp1);

  // in order to inspect variables you have to use the 'stackTrace' command to
  // cause frame ids to be allocated; use the 'scopes' command to cause
  // variables references to be allocated for the constants/globals/locals in
  // scope in a particular frame, and finally the variables command to fetch a
  // variables reference
  $seq = sendVsCommand(darray[
    'command' => 'stackTrace',
    'type' => 'request',
    'arguments' => darray['threadId' => 1],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'stackTrace',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'totalFrames' => 1,
        'stackFrames' => varray[
          darray[
            'id' => 1,
            'source' => darray['path' => $path],
            'line' => $bp1_line,
            'name' => 'main',
          ],
        ],
      ],
    ],
  );

  $seq = sendVsCommand(darray[
    'command' => 'scopes',
    'type' => 'request',
    'arguments' => darray['frameId' => 1]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'scopes',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'scopes' => varray[
          darray[
            'name' => 'Locals',
            'namedVariables' => 1,
            'variablesReference' => 2,
          ],
          darray[
            'name' => 'Superglobals',
            'namedVariables' => 7,
            'variablesReference' => 3,
          ],
          darray[
            'name' => 'Constants',
            'namedVariables' => 2,
            'variablesReference' => 4,
          ],
        ],
      ],
    ],
  );

  // Locals
  $seq = sendVsCommand(darray[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => darray['variablesReference' => 2]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'variables' => varray[
          darray[
            'name' => '$x',
            'type' => 'int',
            'value' => '7',
          ],
        ],
      ],
    ],
  );

  // Superglobals
  $seq = sendVsCommand(darray[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => darray['variablesReference' => 3]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'variables' => varray[
          darray[
            'name' => '$_COOKIE',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          darray[
            'name' => '$_ENV',
            'type' => 'dict',
            // not asserting size of array; ENV may vary
          ],
          darray[
            'name' => '$_FILES',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          darray[
            'name' => '$_GET',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          darray[
            'name' => '$_POST',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          darray[
            'name' => '$_REQUEST',
            'type' => 'dict',
            'namedVariables' => 0,
          ],
          darray[
            'name' => '$_SERVER',
            'type' => 'dict',
            // not asserting size of array as it may vary
          ],
        ],
      ],
    ],
  );

  // Constants; we'll have to chase more variablesReferences from here
  $seq = sendVsCommand(darray[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => darray['variablesReference' => 4]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'variables' => varray[
          darray[
            'name' => 'System Defined Constants',
            'variablesReference' => 11,
            // not asserting number of namedVariables, it will change often
          ],
          darray[
            'name' => 'User Defined Constants',
            'variablesReference' => 10,
            'namedVariables' => 2,
          ],
        ],
      ],
    ],
  );

  // User Defined Constants
  $seq = sendVsCommand(darray[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => darray['variablesReference' => 10]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'variables' => varray[
          darray[
            'name' => 'RUNTIME_INIT',
            'type' => 'int',
            'value' => '2',
          ],
          darray[
            'name' => 'SIMPLE',
            'type' => 'int',
            'value' => '1',
          ],
        ],
      ],
    ],
  );

  // change the value of a local via the REPL
  $seq = sendVsCommand(darray[
    'command' => 'evaluate',
    'type' => 'request',
    'arguments' => darray[
      'frameId' => 1,
      'expression' => '$x = 8;',
    ],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'evaluate',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'type' => 'int',
        'result' => '8',
      ],
    ],
  );

  // set a local via the REPL that doesn't exist in the frame
  $seq = sendVsCommand(darray[
    'command' => 'evaluate',
    'type' => 'request',
    'arguments' => darray[
      'frameId' => 1,
      'expression' => '$y = 99;',
    ],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'evaluate',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'type' => 'int',
        'result' => '99',
      ],
    ],
  );

  // Pull locals again and see changes we made via the REPL
  $seq = sendVsCommand(darray[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => darray['variablesReference' => 2]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'variables' => varray[
          darray[
            'name' => '$x',
            'type' => 'int',
            'value' => '8',
          ],
          darray[
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
  $seq = sendVsCommand(darray[
    'command' => 'stackTrace',
    'type' => 'request',
    'arguments' => darray['threadId' => 1],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'stackTrace',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'totalFrames' => 1,
        'stackFrames' => varray[
          darray[
            'id' => 12,
            'source' => darray['path' => $path],
            'line' => $bp2_line,
            'name' => 'main',
          ],
        ],
      ],
    ],
  );

  $seq = sendVsCommand(darray[
    'command' => 'scopes',
    'type' => 'request',
    'arguments' => darray['frameId' => 12]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'scopes',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'scopes' => varray[
          darray[
            'name' => 'Locals',
            'namedVariables' => 2,
            'variablesReference' => 2,
          ],
          darray[
            'name' => 'Superglobals',
            'namedVariables' => 7,
            'variablesReference' => 3,
          ],
          darray[
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
  $seq = sendVsCommand(darray[
    'command' => 'variables',
    'type' => 'request',
    'arguments' => darray['variablesReference' => 2]
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'variables',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
        'variables' => varray[
          darray[
            'name' => '$x',
            'type' => 'int',
            'value' => '9',
          ],
          darray[
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
  $seq = sendVsCommand(darray[
    'command' => 'evaluate',
    'type' => 'request',
    'arguments' => darray[
      'frameId' => 12,
      'expression' => '$x = 2;',
    ],
  ]);
  $msg = json_decode(getNextVsDebugMessage(), true);
  checkObjEqualRecursively(
    $msg,
    darray[
      'command' => 'evaluate',
      'type' => 'response',
      'request_seq' => $seq,
      'success' => true,
      'body' => darray[
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
