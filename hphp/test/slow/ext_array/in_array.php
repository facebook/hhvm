<?hh

function a() :mixed{
  $os = vec["Mac", "NT", "Irix", "Linux"];
  var_dump(in_array("Irix", $os));
  var_dump(!in_array("mac", $os));
}

function b() :mixed{
  $a = vec["1.10", 12.4, 1.13];
  var_dump(!in_array("12.4", $a, true));
  var_dump(in_array(1.13, $a, true));
}

function c() :mixed{
  $a = vec[vec["p", "h"], vec["p", "r"], "o"];
  var_dump(in_array(vec["p", "h"], $a));
  var_dump(!in_array(vec["f", "i"], $a));
  var_dump(in_array("o", $a));
}


<<__EntryPoint>>
function main_in_array() :mixed{
a();
b();
c();
}
