<?hh

function never<T1 as int, T2 as string super T1>(T1 $_, T2 $_): void {}

function rcvr_bad(
  (function(int,string): void) $_
): void {}

function pass_generic_bad(): void {
  $f = never<>;
  rcvr_bad($f);
}
