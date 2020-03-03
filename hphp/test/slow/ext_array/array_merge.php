<?hh

function a() {
  $array1 = darray[
    "color" => "red",
    0 => 2,
    1 => 4
  ];
  $array2 = darray[
    0 => "a",
    1 => "b",
    "color" => "green",
    "shape" => "trapezoid",
    2 => 4
  ];
  $result = array_merge($array1, varray[$array2]);
  var_dump($result);
}

function b() {
  $array1 = varray[];
  $array2 = darray[1 => "data"];
  $result = array_merge($array1, varray[$array2]);
  var_dump($result);
}

function c() {
  $array1 = varray[];
  $array2 = darray[1 => "data"];
  $result = $array1 + $array2;
  var_dump($result);
}

function d() {
  $beginning = "foo";
  $end = darray[1 => "bar"];
  $result = array_merge((array)$beginning, varray[$end]);
  var_dump($result);
}

function e() {
  $v = 2;
  $a = darray["one" => 1];
  $b = darray["two" => $v];
  $r = array_merge($a, varray[$b]);
  var_dump($r);
}

function f() {
  $id = 100000000000022;
  $a = darray[$id => 1];
  $b = darray[$id => 2];
  $r = array_merge($a, varray[$b]);
  var_dump($r);
}

function g() {
  $a = darray[1 => 50, 5 => 60];
  $b = null;
  var_dump(array_merge($a, varray[$b]));
}


<<__EntryPoint>>
function main_array_merge() {
a();
b();
c();
d();
e();
f();
}
