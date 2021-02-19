<?hh

interface I {
  const FOO = "one";
}

abstract class B {
  const FOO = "two";
}

abstract class A extends B implements I {}
