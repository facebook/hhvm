<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

interface UA {}
class W1 implements UA {}
class W2 implements UA {}

abstract class C {
  abstract const type TA as UA super W1 super W2;
}

class Bad extends C {
  // `W1` sits above `super W1` but not above `super W2`.
  const type TA = W1;
}
