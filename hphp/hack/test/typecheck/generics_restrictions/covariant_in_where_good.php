<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Cov<+T> {
  public function __construct(private T $item) { }
    public function get(): T { return $this->item; }

  public function append1<Tu>(Cov<Tu> $x):Cov<vec<Tu>> where Tu super T {
    return new Cov(vec[$this->get(), $x->get()]);
  }
  public function append2<Tu>(Cov<Tu> $x):Cov<vec<Tu>> where T as Tu {
    return new Cov(vec[$this->get(), $x->get()]);
  }
  public function append3<Tu>(Cov<Tu> $x):Cov<vec<Tu>> where vec<T> as vec<Tu> {
    return new Cov(vec[$this->get(), $x->get()]);
  }
}
