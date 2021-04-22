<?hh

<<__EntryPoint>> function main(): void {
  require_once('test_base.inc');
  init();
  requestAll(
    varray[
      varray[
        'test_proxygen_headers.php',
        null,
        darray[
          'xyzzy' => 42,
          'XyZZy' => 43,
          'XYZZY' => 44,
          'xxxxx' => 45,
        ]
      ]
    ],
  );
}
