<?hh

<<file: __EnableUnstableFeatures('case_types')>>

class C<reify T> {}
class D<T> {}

case type CT0<T> = T;
case type CT1<T> = T;
type TA<T> = T;

function check(mixed $p): void {
  printf("  is C<CT0<int>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<CT0<int>>>($p));
  printf("  is C<CT0<string>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<CT0<string>>>($p));
  printf("  is C<CT0<_>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<CT0<_>>>($p));

  printf("  is C<CT1<int>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<CT1<int>>>($p));
  printf("  is C<CT1<string>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<CT1<string>>>($p));
  printf("  is C<CT1<_>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<CT1<_>>>($p));

  printf("  is C<TA<int>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<TA<int>>>($p));
  printf("  is C<TA<string>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<TA<string>>>($p));
  printf("  is C<TA<_>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<TA<_>>>($p));

  printf("  is C<int> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<int>>($p));
  printf("  is C<string> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<string>>($p));
  printf("  is C<_> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<C<_>>($p));

  printf("  is D<CT0<int>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<CT0<int>>>($p));
  printf("  is D<CT0<string>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<CT0<string>>>($p));
  printf("  is D<CT0<_>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<CT0<_>>>($p));

  printf("  is D<CT1<int>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<CT1<int>>>($p));
  printf("  is D<CT1<string>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<CT1<string>>>($p));
  printf("  is D<CT1<_>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<CT1<_>>>($p));

  printf("  is D<TA<int>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<TA<int>>>($p));
  printf("  is D<TA<string>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<TA<string>>>($p));
  printf("  is D<TA<_>> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<TA<_>>>($p));

  printf("  is D<int> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<int>>($p));
  printf("  is D<string> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<string>>($p));
  printf("  is D<_> = %d\n", \__hhvm_intrinsics\isTypeStructShallow<D<_>>($p));

  printf("\n");
}

<<__EntryPoint>>
function main(): void {
  $p = (new C<CT0<int>>);
  printf("C<CT0<int>>: (reified, case type)\n");
  check($p);

  $p = (new C<TA<int>>);
  printf("C<TA<int>>: (reified, alias)\n");
  check($p);

  $p = new D<CT0<int>>;
  printf("D<CT0<int>>: (non-reified, case type)\n");
  check($p);

  $p = new D<TA<int>>;
  printf("D<TA<int>>: (non-reified, alias)\n");
  check($p);
}
