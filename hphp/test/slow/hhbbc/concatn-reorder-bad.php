<?hh

function f(\HH\string $a, \HH\string $b) {
  return "[".$a.$b."]\n";
}


<<__EntryPoint>>
function main_concatn_reorder_bad() {
echo f('a', 'b');
}
