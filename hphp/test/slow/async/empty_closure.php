<?hh

function main() {
  $x = async function() {};

  $y = $x();
  var_dump(HH\Asio\join($y));
}

<<__EntryPoint>>
function main_empty_closure() {
main();
}
