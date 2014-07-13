//// strict.php
<?hh // strict

function test(): void {
  f1();
  f1(1, 2, 3);

  f2('a');
  f2('a', 'b', 'c');
}

function f1<T>(T ...$args): void {
  foreach ($args as $arg) {
    echo $arg;
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

function takes_keyed_container<Tk,Tv>(KeyedContainer<Tk,Tv> $c): void {}

function takes_vec_array<Tv>(array<Tv> $c): void {}

function takes_hash_array<Tk,Tv>(array<Tk,Tv> $c): void {}

//// partial.php

<?hh // partial

function f2(string $x, ...$args): void {}

class C0 {
  public function meth(...): void {}
}

class C1 extends C0 {
  public function meth(...$args): void {}
}

class C2 extends C1 {
  public function meth($x = null, ...$args): void {}
}

class CH1 extends C0 {
  public function meth(string ...$args): void {}
}

class CH2 extends CH1 {
  public function meth(string $x = 'str', string ...$args): void {}
}
