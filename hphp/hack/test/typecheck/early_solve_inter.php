<?hh

function test(Vector<int> $v): void {
  $m = Map {};

  foreach ($v as $i) {
    if ($m->containsKey("a")) {
      $current_m = $m->get("a");
      if ($current_m !== null) {
        $current_m->add($i);
      }
    } else {
      $m["a"] = Vector {$i};
    }
  }
}
