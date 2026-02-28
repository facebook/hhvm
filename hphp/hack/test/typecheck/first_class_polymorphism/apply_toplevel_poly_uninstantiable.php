<?hh

function never<T1 as int, T2 as string super T1>(T1 $_, T2 $_): void {}

function call_generic_bad(): void {
  $f = never<>;
  $f(3,'3');
}
