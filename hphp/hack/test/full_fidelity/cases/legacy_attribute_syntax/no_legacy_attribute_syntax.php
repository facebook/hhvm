<?hh

<<Attr1, Attr2(5)>> function f(): void {}
<<__Memoize>>
function g<<<__Soft>> reify T>(<<__Soft>> string $_): <<__Soft>> void {}

<<Oncalls('test')>>
class C {
  <<Attr1>> public int $x;
}

g<<<__Soft>> int>('blah');
