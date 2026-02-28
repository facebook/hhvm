//// strict.php
<?hh

function test(): void {
  f1();
  f1(1, 2, 3);
}

function f1<T>(T ...$args): void {
  foreach ($args as $arg) {
    $arg;
  }
  takes_container($args);
  takes_keyed_container($args);
  takes_vec_array($args);
  takes_hash_array($args);
}

function takes_container<T>(Container<T> $arg): ?T {
  foreach ($arg as $elt) {
    return $elt;
  }
  return null;
}

function takes_keyed_container<Tk as arraykey,Tv>(KeyedContainer<Tk,Tv> $c): void {}

function takes_vec_array<Tv>(varray<Tv> $c): void {}

function takes_hash_array<Tk,Tv>(darray<Tk,Tv> $c): void {}
