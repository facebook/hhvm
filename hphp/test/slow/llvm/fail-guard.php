<?hh


function main($i, $j) :mixed{
  return $i + $j;
}

<<__EntryPoint>>
function main_fail_guard() :mixed{
var_dump(main(1, 2));
var_dump(main(1.1, 2.2));
}
