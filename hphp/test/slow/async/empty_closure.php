<?hh

function main() {
  $x = async function() {};

  $y = $x();
  var_dump($y->join());
}
main();
