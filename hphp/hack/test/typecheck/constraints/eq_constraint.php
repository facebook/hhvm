<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Inv<Ti> {}
class B {}
class C<T> {
  public function Foo1<Tu as T super T>(Inv<Tu> $x): Inv<T> {
    return $x;
  }
  public function Foo2<Tu as T super T>(Inv<T> $x): Inv<Tu> {
    return $x;
  }
  public function Boo1<Tu as B super B>(Inv<Tu> $x): Inv<B> {
    return $x;
  }
  public function Boo2<Tu as B super B>(Inv<B> $x): Inv<Tu> {
    return $x;
  }
}
