<?hh

class C {}
class D extends C {}

function redundant_as(D $d): void {
  $d as C;
}

function non_redundant_as(C $c): void {
  $c as D;
}
