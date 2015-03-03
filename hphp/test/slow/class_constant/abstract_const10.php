<?hh // strict

interface I {
  abstract const FOO;
}

interface J {
  const FOO = 1;
}

interface K {
  abstract const FOO;
}

class D implements I, J, K {}

var_dump(D::FOO);
