<?hh

function main($a) {
  $d = dict[&$a, 1, 2, 3];
  var_dump($d);
}
<<__EntryPoint>> function main_entry(): void {
main(1);
}
