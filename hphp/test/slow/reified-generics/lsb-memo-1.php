<?hh

class A {
  <<__MemoizeLSB>>
  static function lsb<reify T>() :mixed{ echo "in lsb\n"; return 1; }
}

<<__EntryPoint>>
function main() :mixed{
  A::lsb<int>();
  A::lsb<string>();
}
