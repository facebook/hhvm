<?hh


function main($o, &$x) {
  $x = $o->prop;
  return isset($x[23]) ? true : false;
}
<<__EntryPoint>> function main_entry(): void {
$o = new stdclass;
$o->prop = array(23 => 'hi');
echo main($o, &$y) ? "true\n" : "false\n";
}
