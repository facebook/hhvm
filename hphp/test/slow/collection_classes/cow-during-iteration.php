<?hh
function dump($iterable) {
  echo get_class($iterable) . "\n";
  foreach ($iterable as $k => $v) {
    echo "$k => $v\n";
  }
}
function dump_set($iterable) {
  echo get_class($iterable) . "\n";
  foreach ($iterable as $v) {
    echo "$v\n";
  }
}
function test() {
  $v = Vector {'a', 'b', 'c'};
  $iv = $v->immutable();
  foreach ($v as $k => $val) {
    if ($k === 1) {
      $v[0] = 'A';
    }
  }
  dump($v);
  $m = Map {0 => 'a', 1 => 'b', 2 => 'c'};
  $im = $m->immutable();
  foreach ($m as $k => $val) {
    if ($k === 1) {
      $m[0] = 'A';
    }
  }
  dump($m);
  $s = Set {'a', 'b', 'c'};
  $is = $s->immutable();
  foreach ($s as $val) {
    if ($val === 'b') {
      $s->add('a');
    }
  }
  dump_set($s);
  $s = Set {'a', 'b', 'c'};
  $is = $s->immutable();
  foreach ($s as $val) {
    if ($val === 'b') {
      $s[] = 'a';
    }
  }
  dump_set($s);
  $m = Map {0 => 'a', 1 => 'b', 2 => 'c'};
  $im = $m->immutable();
  foreach ($m as $k => $val) {
    if ($k === 1) {
      $m->add(Pair {0, 'A'});
    }
  }
  dump_set($m);
}
test();
echo "Done\n";
