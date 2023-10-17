<?hh

type TupleAlias = (int, int);
type ShapeAlias = shape('x' => int);

function check(mixed $p): void {
  var_dump($p);
  printf("  is int = %d\n", \__hhvm_intrinsics\isTypeStructShallow<int>($p));
  printf("  is bool = %d\n", \__hhvm_intrinsics\isTypeStructShallow<bool>($p));
  printf("  is string = %d\n", \__hhvm_intrinsics\isTypeStructShallow<string>($p));
  printf("  is (int,int) = %d\n", \__hhvm_intrinsics\isTypeStructShallow<(int,int)>($p));
  printf("  is TupleAlias = %d\n", \__hhvm_intrinsics\isTypeStructShallow<TupleAlias>($p));
  printf("  is shape('x'=>int) = %d\n", \__hhvm_intrinsics\isTypeStructShallow<shape('x'=>int)>($p));
  printf("  is ShapeAlias = %d\n", \__hhvm_intrinsics\isTypeStructShallow<ShapeAlias>($p));

  printf("\n");
}

<<__EntryPoint>>
function main(): void {
  check(0);
  check(false);
  check('a');
  check(vec[]);
  check(dict[]);
}
