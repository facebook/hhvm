<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}
class E<T> {}
class R<reify T> {}

function concrete(C $x): void {
  hh_show($x);
}

function erased(E<int> $x): void {
  hh_show($x);
}

function reified(R<int> $x): void {
  hh_show($x);
}

function erased_dynamic(E<dynamic> $x): void {
  hh_show($x);
}

function reified_dynamic(R<dynamic> /* currently banned */ $x): void {
  hh_show($x);
}

function erased_like(E<~int> $x): void {
  hh_show($x);
}

function reified_like(R<~int> $x): void {
  hh_show($x);
}
