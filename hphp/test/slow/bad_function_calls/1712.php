<?hh
function foo($x) :mixed{
}
function z() :mixed{
  $yay = 1;
  $snarf = 2;
  foo(1,foo(1), $yay,$snarf);
}


<<__EntryPoint>>
function main_1712() :mixed{
error_reporting(E_ALL & ~E_NOTICE);
z();
}
