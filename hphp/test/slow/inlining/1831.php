<?hh

function h() {
 include '1831.inc';
}
function f($a, $b, $c) {
 return h();
}
function g($a, $b, $c) {
  return f($a++, $b++ + $a++, $c);
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
