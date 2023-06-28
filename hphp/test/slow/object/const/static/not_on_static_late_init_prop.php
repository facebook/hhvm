<?hh

class C {
  <<__Const, __LateInit>> static public int $p = 5;
}

<<__EntryPoint>> function main() :mixed{ echo "FAIL: this file should not parse\n"; }
