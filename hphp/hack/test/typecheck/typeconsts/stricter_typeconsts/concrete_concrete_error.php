<?hh

class C {
  const type T = int;
}

newtype X = int;
class D extends C {
  const type T = X; // can also be Tany
}
