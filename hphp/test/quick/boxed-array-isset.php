<?hh


function main($o, inout $x) {
  $x = $o->prop;
  return isset($x[23]) ? true : false;
}
<<__EntryPoint>> function main_entry(): void {
$o = new stdClass;
$o->prop = darray[23 => 'hi'];
$y = null;
echo main($o, inout $y) ? "true\n" : "false\n";
}
