<?hh

class C {
  const type A = self::B;
  const type B = self::A;
}

function f<reify T>() {
  echo "hello\n";
}

<<__EntryPoint>> function main() {
  f<C::A>();
}
