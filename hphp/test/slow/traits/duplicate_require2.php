<?hh

$n = apc_fetch('foo');
if (!$n) $n = 0;
apc_store('foo', ++$n);

echo "Iteration: $n\n";

if ($n == 1) {
  class A {}
} else if ($n == 2) {
  interface A {}
}

if ($n & 1) {
  trait T1 {
    require extends A;
  }
}

if ($n & 2) {
  trait T2 {
    require implements A;
  }
}

if ($n == 3) {
  class X {
    use T1, T2;
  }
}
