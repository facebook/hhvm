<?hh // strict

abstract class C {
  abstract const FOO;
}

interface I {
  const FOO = 1;
}

class D extends C implements I {}

var_dump(D::FOO);
