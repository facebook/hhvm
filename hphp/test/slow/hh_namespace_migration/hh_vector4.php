<?hh

namespace A {
function test1(): void {
  $x = \HH\Vector {1, 2, 3};
  echo $x->get(0) . "\n";
  echo "---\n";
}
}

namespace B {
function test2(): void {
  $y = \HH\Vector {4, 5, 6};
  for ($i = 0; $i < \count($y); $i++) echo $y[$i] . "\n";
  echo "---\n";
}
}

namespace {
function test3(): void {
  $z = new Vector();
  $z[] = 42;
  echo $z[0] . "\n";
  echo "---\n";
}
}

namespace {
function test4(): void {
  $z = new HH\Vector();
  $z[] = 100;
  echo $z[0] . "\n";
}
}

namespace {
<<__EntryPoint>>
function main(): void {
  A\test1();
  B\test2();
  test3();
  test4();
}
}
