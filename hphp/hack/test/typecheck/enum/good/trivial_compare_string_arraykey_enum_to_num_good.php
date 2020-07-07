<?hh

enum A: string as arraykey {
  X = "X";
}

// Should not fail because A::X is constrained by arraykey (string | int), and ints not trivially unequal to nums
function f1(num $x): bool {
  return A::X === $x;
}

function f2(num $x): bool {
  return $x === A::X;
}

function f3(num $x): bool {
  return A::X !== $x;
}

function f4(num $x): bool {
  return $x !== A::X;
}
