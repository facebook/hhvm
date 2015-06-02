<?hh

function main() {
  $x = async function() {};

  $y = $x();
  var_dump(HH\Asio\join($y));
}
main();
