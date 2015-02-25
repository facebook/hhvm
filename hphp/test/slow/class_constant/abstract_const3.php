<?hh


interface I {
  abstract const X;
  const Y = self::X; // value of X isn't available to the pre-class!
}

class C implements I {
  const X = 'C::X';
}

var_dump(C::X);
var_dump(C::Y);
