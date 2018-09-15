<?hh // strict

class :my-foo<T> {
  attribute vec<T> my-vec;
}

class :my-bar {
  attribute vec<int> my-vec;
}

function test(): void {
  // This is legal, $f is :my-foo<int>
  $f = <my-foo my-vec={vec[1,2,3]} />;
  <my-bar {...$f} />;

  // This is incorrect through the generic
  $f2 = <my-foo my-vec={vec['foo', 'bar', 'baz']} />;
  <my-bar {...$f2} />;
}
