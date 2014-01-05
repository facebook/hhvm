<?hh

namespace A {
  $x = \HH\Vector {1, 2, 3};
  echo $x->get(0) . "\n";
  echo "---\n";
}

namespace B {
  $y = \HH\Vector {4, 5, 6};
  for ($i = 0; $i < count($y); $i++) echo $y[$i] . "\n";
  echo "---\n";
}

namespace {
  $z = new Vector();
  $z[] = 42;
  echo $z[0] . "\n";
  echo "---\n";
}

namespace {
  $z = new HH\Vector();
  $z[] = 100;
  echo $z[0] . "\n";
}
