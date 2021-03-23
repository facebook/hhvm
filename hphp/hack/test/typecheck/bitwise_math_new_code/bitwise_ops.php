<?hh // strict

function good_bitwise_ops(int $a): void {
  1 | 2;
  $a |= 2;
  1 & 2;
  $a &= 2;
  1 ^ 2;
  $a ^= 2;
  1 << 2;
  $a <<= 2;
  1 >> 2;
  $a >>= 2;
  ~1;
}

function bad_bitwise_ops(int $a): void {
  true | false;
  $a |= false;
  1 & false;
  $a &= false;
  "hello" ^ 2;
  $a ^= "hello";
  "hello" << 2;
  $a <<= "hello";
  1 >> "hello";
  $a >>= "hello";
  ~"hello";
}
