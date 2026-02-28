<?hh

function f(\HH\string $a, \HH\string $b) :mixed{
  return "[".$a.$b."]\n";
}


<<__EntryPoint>>
function main_concatn_reorder_bad() :mixed{
echo f('a', 'b');
}
