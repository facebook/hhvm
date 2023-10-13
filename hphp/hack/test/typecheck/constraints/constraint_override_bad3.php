<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class GA<Ta> { }
class GB<Tb1, +Tb2> { }
class GC<Tc> { }
class A extends GB<A, GA<A>> { }
class B { }

class Super<T> {
  // Complicated types
  public function foo<Tf as GA<T>>(GB<Tf, T> $x) : GC<Tf> where T as GB<T, Tf>
  { return new GC(); }
}

// Let's set up a positive test first that matches everything exactly
class Sub extends Super<A> {
  public function foo<Tf as GA<A>>(GB<Tf, A> $x) : GC<Tf> where A as GB<A, Tf>
  { return new GC(); }
}

// Now a negative case
class BadSub<Tz> extends Super<Tz> {
  public function foo<Tf as GA<Tz>>(GB<Tf, Tz> $x) : GC<Tf> where Tz as B
  { return new GC(); }
}
