<?hh

function x() :mixed{
  $y1 = new stdClass;
  $y = new stdClass;
  $y->dyn = $y1;
  unset($y1);
  var_dump($y);
  unset($y);
  var_dump(new stdClass);
}

<<__EntryPoint>>
function main_oid_order() :mixed{
x();
}
