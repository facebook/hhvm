<?hh

function never<T1 as int, T2 as string>(T1 $_, T2 $_): void
where
  bool as int,
  T2 super T1 {}

function call_generic_bad(): void {
  $f = never<>;
  $f(3, '3');
}
