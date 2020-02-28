<?hh <<__EntryPoint>> function main(): void {
$str = "ABCDEFGHIJK";
$arr = varray[null, false, true, 3, 4.0, 5.3, 6.7, -2, -2.5, 21, 22.5,
             PHP_INT_MAX, '', '8', '9a', 'foo', '1 ', ' 2', " \t2", " \n2",
             '999999999999999999999999999', '1.0 ', '2.0 ', '1e1', '1e1 ',
             ' 1e1', varray[], varray[1], new stdclass, 0x100000004];
foreach ($arr as $x) {
  var_dump($x);
  var_dump($str[$x]);
  var_dump(isset($str[$x]));
  echo "\n";
}
}
