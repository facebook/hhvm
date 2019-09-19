<?hh

function f<Tf as (function (int): num)>(dynamic $d): void {
  $d as ~Tf; // TODO
}

function g<Tg super (function (num): int)>(dynamic $d): void {
  $d as ~Tg; // TODO
}

function h<Th>(dynamic $d): void {
  $d as ~Th;
}
