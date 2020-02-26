//// file1.php
<?hh // partial

/* HH_FIXME[4101] */
function foo(MyMap $m) {
  $m->add("hey", 40);
  return $m->get("ho");
}

//// file2.php
<?hh

class MyMap<Tk, Tv> {
  public function add(Tk $x, Tv $y): void {}
  public function get(Tk $x): ?Tv {
    return null;
  }
}
