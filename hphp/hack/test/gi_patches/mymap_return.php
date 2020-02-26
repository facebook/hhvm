//// file1.php
<?hh // partial

/* HH_FIXME[4101] */
function foo(): MyMap {
  $m = new MyMap();
  $m->add("hey", 40);
  return $m;
}

//// file2.php
<?hh

class MyMap<Tk, Tv> {
  public function add(Tk $x, Tv $y): void {}
  public function get(Tk $x): ?Tv {
    return null;
  }
}

//// file3.php
<?hh

function bar(): void {
  expect<MyMap<string, int>>(foo());
}

function expect<T>(T $_): void {}
