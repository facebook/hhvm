<?hh // strict

function foo(vec<int> $v, dynamic $d): void {
  $v[$d] = 5;
  hh_show($v);
}

function bar(dict<int, int> $v, dynamic $d): void {
  $v[$d] = 5;
  hh_show($v);
}

function baz(dynamic $d): void {
  $dic1 = dict[];
  $dic1[5] = 5;
  $dic1[$d] = 5;
  hh_show($dic1);
  $dic2 = dict[];
  $dic2[$d] = 5;
  $dic2[5] = 5;
  hh_show($dic2);
}
