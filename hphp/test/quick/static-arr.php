<?hh


<<__EntryPoint>> function main(): void {
  $x = array();
  $x += varray[1, 2, 3];
  var_dump($x);

  $s = 'hi';
  $s .= '1234556';
  var_dump($s);
}
