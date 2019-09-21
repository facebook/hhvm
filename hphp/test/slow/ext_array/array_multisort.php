<?hh

function a() {
  $ar1 = array(10, 100, 100, 0);
  $ar2 = array(1, 3, 2, 4);
  array_multisort2(inout $ar1, inout $ar2);
  var_dump($ar1);
  var_dump($ar2);
}

function b() {
  $ar0 = array("10", 11, 100, 100, "a");
  $ar1 = array(1, 2, "2", 3, 1);
  $asc = SORT_ASC;
  $string = SORT_STRING;
  $numeric = SORT_NUMERIC;
  $desc = SORT_DESC;
  array_multisort6(inout $ar0, inout $asc, inout $string, inout $ar1, inout $numeric, inout $desc);
  $ar = array(
    $ar0,
    $ar1,
  );
  var_dump($ar);
}

function c() {
  $array = array("Alpha", "atomic", "Beta", "bank");
  $array_lowercase = array_map(fun("strtolower"), $array);
  $asc = SORT_ASC;
  $string = SORT_STRING;
  array_multisort4(inout $array_lowercase,
                   inout $asc, inout $string, inout $array);
  var_dump($array);
}



<<__EntryPoint>>
function main_array_multisort() {
a();
b();
c();
}
