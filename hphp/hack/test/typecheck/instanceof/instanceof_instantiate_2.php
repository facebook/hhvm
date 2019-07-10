<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface J<+Tj> { }
interface I<+Ti as J<Ti>> { }
class Cov<+T as I<T> as J<T>> { }

class F implements J<F>, I<F> { }

function istest(mixed $m): void {
  if ($m is Cov<_>) {
    hh_show($m);
  }
}
function callit():void {
  istest(new Cov<F>());
}
