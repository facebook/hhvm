<?hh

function x() :mixed{
  $y1 = new stdClass;
  $y = new stdClass;
  $y->dyn = $y1;
  $y1 = null;
  var_dump($y);
  $y = null;
  var_dump(new stdClass);
}

<<__EntryPoint>>
function main_oid_order() :mixed{
x();
}
