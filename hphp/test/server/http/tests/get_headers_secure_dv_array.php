<?hh

require_once('test_base.inc');

function opts(bool $hack_arr = false): string {
  $args = varray[
    '-vEval.HackArrCompatTypeHintNotices=1',
  ];
  if ($hack_arr) {
    $args[] = '-vEval.HackArrDVArrs=true';
  }
  return implode(' ', $args);
}

function hphp_opts(bool $hack_arr = false): string {
  $args = varray[
    '-vRuntime.Eval.HackArrCompatTypeHintNotices=1',
  ];
  if ($hack_arr) {
    $args[] = '-vHackArrDVArrs=1';
  }
  return implode(' ', $args);
}

<<__EntryPoint>> function main(): void {
  requestAll(
    varray[
      varray[
        'test_get_headers_secure.php',
        null,
        darray[
          'xyzzy' => 42,
          'XyZZy' => 43,
          'XYZZY' => 44,
          'xxxxx' => 45,
        ]
      ]
    ],
    opts(),
    hphp_opts(),
  );
  requestAll(
    varray[
      varray[
        'test_get_headers_secure.php',
        null,
        darray[
          'xyzzy' => 42,
          'XyZZy' => 43,
          'XYZZY' => 44,
          'xxxxx' => 45,
        ]
      ]
    ],
    opts(true),
    hphp_opts(true),
  );
}
