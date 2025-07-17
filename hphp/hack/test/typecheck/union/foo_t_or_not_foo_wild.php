<?hh

interface IMyBase<+Tcov, Tinv> {}
interface IMyA<+Tcov, Tinv> extends IMyBase<Tcov, Tinv> {}
interface IMyB<+Tcov, Tinv> extends IMyBase<Tcov, Tinv> {}

function repro_helper(IMyB<mixed, int> $_): void {}

function repro_fun(IMyBase<mixed, int> $x): void {
  if ($x is IMyA<_, _>) {
  }
  if ($x is IMyB<_, _>) {
    repro_helper($x);
  }
}
