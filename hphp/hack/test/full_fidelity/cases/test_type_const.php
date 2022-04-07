<?hh
class C {
  const type T1 = int;
  const type T2 as T1 = T1;
}

abstract class A {
  abstract const type T0;
  abstract const type T1 as string;
}
