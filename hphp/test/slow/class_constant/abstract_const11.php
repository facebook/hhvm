<?hh // strict

interface I {
  abstract const FOO;
}

interface J {
  const FOO = 1;
}

interface K {
  const FOO = 1;
}

class D implements I, J, K {}
