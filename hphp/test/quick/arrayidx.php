<?hh

<<__EntryPoint>> function main(): void {

  $a = darray['1' => '2', 'hello' => 'world', '' => 'empty'];
  $b = null;

  var_dump(hphp_array_idx($a, '1', 3));
  var_dump(hphp_array_idx($a, '0', 4));
  var_dump(hphp_array_idx($a, 1, 5));
  var_dump(hphp_array_idx($a, 0, 6));

  var_dump(hphp_array_idx($a, 'hello', 10));
  var_dump(hphp_array_idx($a, 'world', 11));
  var_dump(hphp_array_idx($a, '', 12));
  var_dump(hphp_array_idx($a, null, 13));

  // should fatal
  var_dump(hphp_array_idx($b, 'not_reached', 14));
}
