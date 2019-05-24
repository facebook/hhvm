<?hh


function main($a, &$x) {
  $x = $a[0];
  return (!($x ?? false)) ? true : false;
}
<<__EntryPoint>> function main_entry(): void {
echo main(array(array()), &$y)."\n";
}
