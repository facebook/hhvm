<?hh

class A {
  <<__MemoizeLSB>>
  static function lsb<reify T>() { echo "in lsb\n"; return 1; }
}

<<__EntryPoint>>
function main() {
  A::lsb<int>();
  A::lsb<string>();
}
