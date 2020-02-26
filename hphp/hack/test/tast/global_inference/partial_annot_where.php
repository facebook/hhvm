//// file1.php
<?hh // partial

/* HH_FIXME[4101] */
function foo(A $x) {
  return $x->get();
}

//// file2.php
<?hh

class A<T super string> {
  public function get(): ?T where T super arraykey {
    return null;
  }
}
