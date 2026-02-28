<?hh

function f(shape('bar' => vec<int>) $s): void {
  $s['bar'][0] ?? null;
  $s['bar'] ?? null;
  $s['bar'];
}

function g(shape(?'bar' => vec<int>) $s): void {
  $s['bar'][0] ?? null;
  $s['bar'] ?? null;
  $s['bar'];
}

function h(shape() $s): void {
  $s['bar'][0] ?? null;
  $s['bar'] ?? null;
  $s['bar'];
}
