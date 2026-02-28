<?hh

<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  requestAll(
    vec[
      vec[
        'test_unsetting_headers.php',
        null,
        dict[
          'MyHeader' => 42,
        ]
      ]
    ],
  );
  requestAll(
    vec[
      vec[
        'test_unsetting_headers.php',
        null,
        dict[
          'MyHeader' => 42,
        ]
      ]
    ],
    '-vEval.SetHeadersInServerSuperGlobal=0'
  );
}
