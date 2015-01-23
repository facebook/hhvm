<?hh

abstract class C1 {
  abstract const X;
  const Y = 10;
}

class C2 extends C1 {
  const X = 20;
}
