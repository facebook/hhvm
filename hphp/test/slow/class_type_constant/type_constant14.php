<?hh // strict

interface I {
  const type T = int;
}

class C implements I {
  const type T = string;
}
