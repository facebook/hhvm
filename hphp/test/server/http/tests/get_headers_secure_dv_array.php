<?hh

<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  requestAll(
    vec[
      vec[
        'test_get_headers_secure.php',
        null,
        dict[
          'xyzzy' => 42,
          'XyZZy' => 43,
          'XYZZY' => 44,
          'xxxxx' => 45,
        ]
      ]
    ],
    '',
    ''
  );
}
