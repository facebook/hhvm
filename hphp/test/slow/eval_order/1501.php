<?hh

function main() {
  $a = varray[123];
  foreach ($a as $x => $x) {
    var_dump($x);
  }
}


<<__EntryPoint>>
function main_1501() {
main();
}
