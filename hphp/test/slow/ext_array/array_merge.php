<?hh

function a() :mixed{
  $array1 = dict[
    "color" => "red",
    0 => 2,
    1 => 4
  ];
  $array2 = dict[
    0 => "a",
    1 => "b",
    "color" => "green",
    "shape" => "trapezoid",
    2 => 4
  ];
  $result = array_merge($array1, vec[$array2]);
  var_dump($result);
}

function b() :mixed{
  $array1 = vec[];
  $array2 = dict[1 => "data"];
  $result = array_merge($array1, vec[$array2]);
  var_dump($result);
}

function c() :mixed{
  $array1 = vec[];
  $array2 = dict[1 => "data"];
  $result = array_merge($array1, $array2);
  var_dump($result);
}

function d() :mixed{
  $beginning = "foo";
  $end = dict[1 => "bar"];
  $result = array_merge(dict[0 => $beginning], vec[$end]);
  var_dump($result);
}

function e() :mixed{
  $v = 2;
  $a = dict["one" => 1];
  $b = dict["two" => $v];
  $r = array_merge($a, vec[$b]);
  var_dump($r);
}

function f() :mixed{
  $id = 100000000000022;
  $a = dict[$id => 1];
  $b = dict[$id => 2];
  $r = array_merge($a, vec[$b]);
  var_dump($r);
}

function g() :mixed{
  $a = dict[1 => 50, 5 => 60];
  $b = null;
  var_dump(array_merge($a, vec[$b]));
}


<<__EntryPoint>>
function main_array_merge() :mixed{
a();
b();
c();
d();
e();
f();
}
