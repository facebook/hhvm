<?hh

interface I {
  abstract const X;
}

trait Tr1 implements I {}

trait Tr2 {
  abstract const X;
}
