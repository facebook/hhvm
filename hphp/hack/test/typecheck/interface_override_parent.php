<?hh

interface I1 {
  const FOO1 = "interface";
}

class B {
  const FOO1 = "parent";
}

class A extends B implements I1 {}
