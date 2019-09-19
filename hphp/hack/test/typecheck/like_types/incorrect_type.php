<?hh

function f(HH\INCORRECT_TYPE<int> $i, dynamic $d): void {
  hh_show($i);

  $d as HH\INCORRECT_TYPE<string>;
  hh_show($d);
}

function bad(dynamic $d): void {
  $d as HH\INCORRECT_TYPE<(function (int): void)>;
  hh_show($d);
}
