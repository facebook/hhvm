<?hh

interface I {
  const FOO = 42;
}

trait T1 implements I {
}

trait T {
  const DEFAULT = "baseline";
}


class C {
  use T1;
  use T;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C::FOO);
  var_dump(C::DEFAULT);
}
