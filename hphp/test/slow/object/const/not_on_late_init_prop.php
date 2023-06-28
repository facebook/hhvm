<?hh

class C {
  <<__Const, __LateInit>> public int $p;
}

<<__EntryPoint>> function main() :mixed{ echo "FAIL: this file should not parse\n"; }
