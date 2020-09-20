<?hh // strict

class A {}
class B {}
class C {}
class D {}
class E {}
class F {}
class G {}
class H {}
class I {}
class J {}
class K {}

// This test attempts to make sure continuations within a lambda don't
// interfere with continuations outside

function f(bool $b): (F, F) {
  $x = new A();
  $y = new A();

  while ($b) {
    hh_show($x); // A | B | C
    hh_show($y); // A | B | C
    $x = new B();
    $y = new B();
    if ($b) {
      $x = new C();
      $y = new C();
      continue;
    }
    if ($b) {
      $x = new E();
      $y = new E();
      break;
    }
    if ($b) {
      $x = new F();
      $y = new F();
      return tuple($x, $y);
    }

    $f = (
      $x ==> {
        if ($b) {
          $x = new K();
          $y = new K();
          continue;
        }
        while ($b) {
          hh_show($x); // _ | G | J | A
          //hh_show($y); // B | G | J // // TODO: uncomment when naming is fixed
          if ($b) {
            $x = new G();
            $y = new G();
            continue;
          }
          if ($b) {
            $x = new D();
            $y = new D();
            break;
          }
          if ($b) {
            $x = new H();
            $y = new H();
            return tuple($x, $y);
          }
          $x = new J();
          $y = new J();
        }
        hh_show($x); // _ | G | D | J | A
        // hh_show($y); // B | G | D | J // TODO: uncomment when naming is fixed
        return tuple(new H(), new H());
      }
    );

    expect_B($x);
    expect_B($y);
    list($a, $b) = $f(new A());
    expect_H($a);
    expect_H($b);
    expect_B($x);
    expect_B($y);
  }

  hh_show($x); // A | B | C | E
  hh_show($y); // A | B | C | E
  return tuple(new F(), new F());
}

function expect_B(B $x): void {}
function expect_H(H $X): void {}
function foo((function(C): void) $f): void {}
