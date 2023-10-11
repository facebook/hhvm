<?hh

class C1 {}

class C2 extends C1 {}

class C3 extends C2 {}

function f(inout C2 $c): void {
  switch (mt_rand() % 2) {
    case 0:
      $c = new C2();
      break;
    case 1:
      $c = new C3();
      break;
  }
}

function g(inout C1 $c): void {
  switch (mt_rand() % 3) {
    case 0:
      $c = new C1();
      break;
    case 1:
      $c = new C2();
      break;
    case 2:
      $c = new C3();
      break;
  }
}

function test(): void {
  $c = new C3();
  f(inout $c);
  f(inout $c);
  f(inout $c);
  g(inout $c);
  f(inout $c);
}
