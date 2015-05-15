<?php

class X {
  const Y = 42001;
}

class A {
  static function f() {
    return Map {
        'a' => Vector { 1 },
        'b' => Vector { 1 },
        'c' => Vector { 1 },
        'd' => Vector { 1 },
        'e' => Vector { 1 },
        'f' => Vector { 1 },
        'g' => Vector { 1 },
        'h' => Vector { 1 },
        'i' => Vector { 1 },
        'j' => Vector { 1 },
        'k' => Vector { 1 },
        'l' => Vector { 1 },
        'm' => Vector { 1 },
        'n' => Vector { 1 },
        'o' => Vector { 1 },
        'p' => Vector { 1 },
        'q' => Vector { 1 },
        'r' => Vector { 1 },
        's' => Vector { 1 },
        't' => Vector { 1 },
        'u' => Vector { 1 },
        'v' => Vector { 1 },
        'w' => Vector { 1 },
        'x' => Vector { 1 },
        'y' => Vector { 1 },
        'z' => Vector { 1 },
        'a0' => Vector { 1 },
        'a1' => Vector { 1 },
        'a2' => Vector { 1 },
        'a3' => Vector { 1 },
        'a4' => Vector { 1 },
        'a5' => Vector { 1 },
        'a6' => Vector { 1 },
        'a7' => Vector { 1 },
        'a8' => Vector { 1 },
        'a9' => Vector { 1 },
        'b0' => Vector { 1 },
        'b1' => Vector { 1 },
        'b2' => Vector { 1 },
        'b3' => Vector { 1 },
        'b4' => Vector { 1 },
        'b5' => Vector { 1 },
        'b6' => Vector { 1 },
        'b7' => Vector { 1 },
        'b8' => Vector { 1 },

        'b9' => Vector {
          X::Y,
        },
      };
    }
}

for ($i = 0; $i < 5; $i++) {
  $m = A::f();
  var_dump($m['b9'][0]);
}
