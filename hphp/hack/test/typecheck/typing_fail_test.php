<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/*
int function toto2(hashtable(string, int) $h) {
  $x = "";
  $y = 0;
  if($x == "xx") {
  }
  else {
    $x = 9;
  }
  $h['foo'] = 0;
  return 0;
}
*/
/*
Error argument already bound
num function my_plus(num $x, num $x) {
}
 */

/*
Error float and int
int function my_plus(num $x, num $y) {
  return $x + $y + 1.0;
}
*/

/*
int function f(int $x) {
  $y = null;
  $x = $y;
}
*/

/*
int function f(option(int) $y) {
  if($y) {
    return $y+1;
  } else {
  }
}
*/

/*
int function f(option(int) $y) {
  if($y) {
    return $y+1;
  } else {
  }
  return 0;
}
*/

/*
class X {

  private $x;

  function __construct() {
  }

  int function get() {
    return $this->x;
  }
}

*/

/*
int function f(int $n, int $y = 0) {
  if($n == 0) {
    return 1;
  }
  return f(0);
}
*/
/*
int function get(option(int) $x) {
  if($x) {
    return $x;
  }
  return 0;
}
*/
/*
T function id(T $x) {
  return $x;
}

string function test() {
  $x = id(0);
  $y = id("dfa");
  return $y;
}
*/


/*
class C<T> {

  public T $x;

  public function __construct(T $x) {
    $this->x = $x;
  }

  public T function set_x(T $x) {
    $this->x = $x;
    return $x;
  }

  public T function get_x() {
    return $this->x;
  }
}

int function f() {
  $x = new C();
  $y = $x->set_x(22);
  $c2 = new C();
  $z = $c2->get_x();
  return $z;
}

int function f2(option(int) $x) {
  if($x) {
    return $x;
  }
  else {
    return 0;
  }
}
*/
T function show(T $x) { return $x; }

/*
class A {
  public option(int) $x;

  public function __construct() {
    $this->x = some(0);
  }

  public bool function get_x() {
    $x = $this->x;
    if ($x) {
    }
    $z = $x + 1;
    return true;
  }

}
*/

/*
//testing unify_fun_strict
class A {
  public int function f(bool $x = true, bool $h = true) {
    return 0;
  }
}

class B {
  public int function f(bool $x, bool $y = true) {
    return 1;
  }
}

int function x() {
  $x = new A();
  if(true) {
    $x = new B();
  }
  return $x->f();
}
*/

/*
class A {
  public bool function f(A $x) { return true; }
}

class B {
}

bool function test() {
  $a = new A();
  $b = new B();
  $u = $a->f($b);
  return false;
}
*/

/*
class A {

  public A function f(A $x) {
    if(true) {
      return $x;
    }
    else {
      return $x->f(new A());
    }
  }

}
*/

/*
class A {
  public A function f(A $x) { return $x; }
}

class B {
  public B function f(B $x) { return $x; }
  public B function g(B $x) { return $x; }
}

bool function test() {
  $a = new A();
  if(true) {
    $a = new B();
  }
  $z = $a->f($a);
  $z = $z->g($z);
  return false;
}
*/


/*
class A {
  public A function f(A $x) {
    $z = show($x);
    return $x;
  }
}


class B {
  public B function f(B $x) { return $x; }
  public B function g(B $x) { return $x; }
}


bool function test() {
  $a = new A();
  $x = $a->f($a);
  $uu = show($x);
  $uuu = $a->g($x);
  return false;
}
*/

/*
class L<T> {
  private T $v;

  public function __construct(T $x) {
    $this->v = $x;
  }

  public bool function append(T $x) {
    $this->v = $x;
    return true;
  }

  public T function get() {
    return $this->v;
  }
}
*/

/*
class A {
  bool function f(bool $x) { return true; }
}

class B {
  bool function f(bool $x) { return true; }
  bool function g(bool $x) { return true; }
}

class C {
  bool function f(bool $x) { return true; }
  bool function h(bool $x) { return true; }
}




bool function justA(A $x) { return true;}
bool function justC(C $x) { return true;}

bool function tt() {
  $y = Map {'f00' => new A()};
  $t = tuple(1, new A());
  $v = Vector{};
  $v[0] = 0.0;
  $x = $y['f00'];
  if(true) {
    $x = $t[1];
  }
  return true;
}
*/

/*

bool function test(L<A> $l) {
//  $zz = $l->append(new C());
  return true;
}


class XX {

  B function test44(L<A> $l) {

    $zz = justC(new A());

    return new B();
  }
}
*/

/*
bool function xx(A $x) {
  return true;
}

bool function yy() {
  $x = new B();
  $zz = xx($x);
  $zz = $x->g(true);
  return true;
}
*/

/*
bool function test2() {
  $x = test(new L());
  return $x->get()->f(true);
}
*/

/*
class A<T> {
  public A<T> function f(T $x) {
    return $this->f(new A());
  }
}
*/

/*
class A<T1, T2> {
  public A<T2, T1> function f(A<T1, T2> $x) {
    return $x;
  }
}
*/

/*
class A<T1, T2> {

  public T1 function f(T1 $x) {
    return $x;
  }
  public T2 function g(T2 $x) {
    return $x;
  }
}

class B {
  public float function f(int $x) {
    return 5.1;
  }
  public bool function g(bool $x, int $y = 0) {
    return $x;
  }
}

class C<T1, T2> {
  public T2 function f(T2 $x) {
    return $x;
  }

  public T1 function g(T1 $x) {
    return $x;
  }
}

bool function test3() {
  return (new C() == new A());
}
*/



/*
bool function test2() {
  $b = new B();
  $a = new A();
  if(true) {
    $b = $a;
  }
  $z = $b->g(true, 1);
  return true;
}
*/

/*
option(T) function f(T $x) {
  return f($x);
}
*/

/*
bool function f() {
  $x = 1.0;

  $z = 0;

  $y = 0.0;

  if (true) {
    $y = $x;
  } else {
    $y = $z;
  }

  return $y;
}
*/

/*
class C1 {
}

class C2 {
  public int function f() { return 0; }
}

class A {
  option(C1) function f(bool $x) { return null; }
}

class B {
  option(C2) function f(bool $x, bool $y = true) { return null; }
  bool function g(bool $x) { return true; }
}

bool function test(A $x, int $y) {
  return true;
}

bool function test2() {
  $b = new B();
  $z = test($b, 1);
  $z = show($b);
  return true;
}


*/

/*
class A {
  bool function f(bool $x) { return true; }
}

class B {
  bool function f(bool $x) { return true; }
  bool function g(bool $x) { return true; }
}


class XX {

  public B $x;

  public function __construct(A $x) {
    $this->x = $x;
  }
}


bool function f(bool $x) {
  $y = new A();
  $y->f(true);
  return true;
}
*/

/*
class A {
  public A function f(A $x) { return new A(); }
}

class B {
  public A function f(A $x) { return new A(); }
  public B function g(B $x) { return $x; }
}

class XX {

  public B $x;

  public function __construct(B $x) {
    $this->x = $x;
  }

  public bool function onstruct(A $x) {
    $zz = $x->f($this->x);
    $this->x = $zz;
    return true;
  }
}

*/
/*
class C1 {
}

class C2 {
  public int function f() { return 0; }
}

class A {
  C1 function f(C2 $x) { return $x; }
}

class B {
  C1 function f(C2 $x) { return $x; }
  bool function g(bool $x) { return true; }
}

bool function test(A $x, int $y) {
  return true;
}

A function test2(B $x) {
  $b = $x;
  if(true) { $b = new A(); }
  $zz = show($b);
  return $b;
}
*/


class A<T> {

  private A<T> function f(T $x) {
    $y = new A();
    return $y;
  }
}

// parameter substitution
class C<T> extends A<T> {
  public T function get(T $x) {
    $y = new A();
    $this->f($x);
    return $x;
  }
  private A<T> function f(T $x) {
    return new A();
  }
}

int function test() {
  $x = new C();
  return $x->get(0);
}

