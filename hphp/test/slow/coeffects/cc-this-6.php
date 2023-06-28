<?hh

abstract class A1 {
  abstract const ctx C1;
  abstract const ctx C2 super [defaults];
  const ctx C3 = [rx];
}

interface I1 {
  abstract const ctx C1;
  abstract const ctx C2 super [defaults];
  const ctx C3 = [rx];
}

class TestCls1 {
  const type T1 = A1;
  const type T2 = I1;
  function f()[this::T1::C1, this::T1::C2, this::T1::C3] :mixed{}
  function g()[this::T2::C1, this::T2::C2, this::T2::C3] :mixed{}
}

<<__EntryPoint>>
function main()[] :mixed{
  $c = new TestCls1();
  $c->f();
  $c->g();
}
