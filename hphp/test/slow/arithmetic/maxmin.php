<?hh

function main($a, $b) :mixed{
  var_dump(min($a, $b), max($a, $b));
  }


<<__EntryPoint>>
function main_maxmin() :mixed{
main(1.0, 1);
main(1, 1.0);
}
