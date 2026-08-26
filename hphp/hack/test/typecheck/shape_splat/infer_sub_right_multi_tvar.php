<?hh
<<file:
  __EnableUnstableFeatures(
    'shape_splat_concrete',
    'shape_splat_type_parameters',
    'shape_splat_expression',
  )>>

interface I {
  public function combine<T1 as shape(...), T2 as shape(...)>(
    T1 $a,
    T2 $b,
  ): shape(...T1, ...T2);
}

function sink(shape('x' => int, 'y' => string) $s): void {}

function bad(I $i): void {
  sink($i->combine(shape('x' => 1), shape('y' => 'hi')));
}

function ok(I $i): void {
  $x = $i->combine(shape('x' => 1), shape('y' => 'hi'));
  sink($x);
}

function combine<T1 as shape(...), T2 as shape(...)>(
  T1 $a,
  T2 $b,
): shape(...T1, ...T2) {
  throw new Exception();
}

function bad_too(): void {
  sink(combine(shape('x' => 1), shape('y' => 'hi')));
}

function ok_too(): void {
  $x = combine(shape('x' => 1), shape('y' => 'hi'));
  sink($x);
}
