<?hh

function a() {
  $os = varray["Mac", "NT", "Irix", "Linux"];
  var_dump(in_array("Irix", $os));
  var_dump(!in_array("mac", $os));
}

function b() {
  $a = varray["1.10", 12.4, 1.13];
  var_dump(!in_array("12.4", $a, true));
  var_dump(in_array(1.13, $a, true));
}

function c() {
  $a = varray[varray["p", "h"], varray["p", "r"], "o"];
  var_dump(in_array(varray["p", "h"], $a));
  var_dump(!in_array(varray["f", "i"], $a));
  var_dump(in_array("o", $a));
}


<<__EntryPoint>>
function main_in_array() {
a();
b();
c();
}
