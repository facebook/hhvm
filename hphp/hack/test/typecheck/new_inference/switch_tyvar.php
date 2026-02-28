<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum Foo: string {
  BAR = 'bar';
  BAZ = 'baz';
}

function dict_map<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): dict<Tk, Tv2> {
  return dict[];
}

function return_something(keyset<Foo> $foo): void {
  dict_map(
    $foo,
    $source ==> {
      switch ($source) {
        case Foo::BAR:
          return null;
        case Foo::BAZ:
          return null;
      }
    },
  );
}
