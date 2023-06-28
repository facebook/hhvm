<?hh

function h() :mixed{
 include '1831.inc';
}
function f($a, $b, $c) :mixed{
 return h();
}
function g($a, $b, $c) :mixed{
  return f($a++, $b++ + $a++, $c);
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
