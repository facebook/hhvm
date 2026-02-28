<?hh

<<__EntryPoint>>
function main(): void {
  $bad_utf8 = quoted_printable_decode('=B0');

  $error = null;
  json_encode_with_error($bad_utf8, inout $error);
  var_dump($error[0], $error[1]);

  $a = new stdClass;
  $a->foo = quoted_printable_decode('=B0');
  $error = null;
  json_encode_with_error($a, inout $error);
  var_dump($error[0], $error[1]);

  $b = new stdClass;
  $b->foo = $bad_utf8;
  $b->bar = 1;
  $error = null;
  json_encode_with_error($b, inout $error);
  var_dump($error[0], $error[1]);

  $c = dict[
      'foo' => $bad_utf8,
      'bar' => 1
  ];
  $error = null;
  json_encode_with_error($c, inout $error);
  var_dump($error[0], $error[1]);
}
