<?hh

<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  requestAll(
    vec[
      vec[
        'test_unsetting_cookies.php',
        null,
        dict[
          'Cookie' => "Key1=Value1;Key2=Value2",
        ]
      ]
    ],
  );
  requestAll(
    vec[
      vec[
        'test_unsetting_cookies.php',
        null,
        dict[
          'Cookie' => "Key1=Value1;Key2=Value2",
        ]
      ]
    ],
    '-vEval.DisableParsedCookies=1'
  );
}
