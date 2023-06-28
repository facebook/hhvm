<?hh

class C {
  const type A = self::B;
  const type B = self::A;
}

function f<reify T>() :mixed{
  echo "hello\n";
}

<<__EntryPoint>> function main() :mixed{
  f<C::A>();
}
