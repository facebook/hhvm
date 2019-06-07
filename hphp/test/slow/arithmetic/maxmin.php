<?hh

function main($a, $b) {
  var_dump(min($a, $b), max($a, $b));
  }


<<__EntryPoint>>
function main_maxmin() {
main(1.0, 1);
main(1, 1.0);
}
