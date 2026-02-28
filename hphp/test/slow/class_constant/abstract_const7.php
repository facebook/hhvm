<?hh

interface I {
  abstract const X;
}

trait Tr1 implements I {}

trait Tr2 {
  abstract const X;
}

class C {
  use Tr2;
  const X = 10;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C::X);
}
