<?hh
function foo($x) {
}
function z() {
  $yay = 1;
  $snarf = 2;
  foo(1,foo(1), $yay,$snarf);
}


<<__EntryPoint>>
function main_1712() {
error_reporting(E_ALL & ~E_NOTICE);
z();
}
