<?hh // strict

// This is a more thorough test of the expression dependent type
// produced by the expression 'new Class()'

<<__ConsistentConstruct>>
class C {
  const type T as arraykey = arraykey;

  public function get(): this::T {
    // UNSAFE
  }

  public static function takesThis(this $c): void {}

  protected function set(this::T $x): void {}

  public function test(): void {
    // new C() will produce the dependent type `cls "\C"
    $c = new C();
    hh_show($c);

    // When we resolve this::T when invoking set(), $c is known
    // to have the late bound type of C, so this::T is resolved to
    // C::T and we choose the assigned type. This means any string or
    // int will be accepted.
    $c->set('');
    $c->set(0);

    // Additionally the type 'this' in C::takesThis(), will be resolved to
    // the dependent type `cls "\C", so calling $c does not produce an error
    C::takesThis($c);

    // new self() will do the same thing as new C() above
    $c = new self();
    hh_show($c);
    $c->set('');
    $c->set(0);
    C::takesThis($c);
  }
}

class D extends C {
  const type T = int;

  public function test2(bool $cond): void {
    if ($cond) {
      // $x has dependent type `cls "\C"
      $x = new C();
    } else if ($cond) {
      // $x has dependent type `cls "\D"
      $x = new D();
    } else {
      // $x has dependent type `cls "\D"
      $x = new self();
    }
    // After integration, $x is an unresolved type of
    // Tunresolve[`cls "\C", `cls "\D"]
    hh_show($x);
    // this::T is resolved as C::T, and D::T in case of unresolved type. Since
    // we know the exact type in this case, we use the assigned type. This means
    // C::T -> arraykey, and D::T -> int so the int 0 is a valid input.
    $x->set(0);

    // The same remains true when we reassing $x to another local
    $y = $x;
    $y->set(0);

    // If $x is nullable we still resolve the type constants correctly
    if ($cond) {
      $x = null;
    }
    hh_show($x);
    $x?->set(0);
  }

  public function test3(): void {
    // new parent() is resolved to new C(), so it has dependent type `cls "\C"
    // This means it passes all the cases in C::test()
    $parent = new parent();
    hh_show($parent);
    $parent->set('');
    $parent->set(0);
    C::takesThis($parent);

    // new self() is resolved to new D(), so it has dependent type `cls "\D"
    $d = new self();
    hh_show($d);
    $d->set(0);
  }
}

function test_vec_unpack(): (int, int) {
  // Make sure list() assign still works even though new Vector()
  // produces a dependent type
  list($x, $y) = new Vector();
  return tuple($x, $y);
}
