<?hh

class C {
  public $bar;
}

function test_regular(mixed $recursive_thing): void {
  $foo = new C();
  $foo->bar = $recursive_thing;
  objprof_get_strings(10);
}

function test_vector(mixed $recursive_thing): void {
  $foo = Vector{$recursive_thing};
  objprof_get_strings(10);
}

function test_pair(mixed $recursive_thing): void {
  $foo = Pair{0, $recursive_thing};
  objprof_get_strings(10);
}

function test(string $name, mixed $recursive_thing): void {
  echo "---- $name ----\n";
  test_regular($recursive_thing);
  test_vector($recursive_thing);
  test_pair($recursive_thing);
}

function main(): void {
  $r = varray[];
  test("shallow array", $r);

  $r = varray[varray[]];
  test("deep array", $r);

  echo "---- the end ----\n";
}


<<__EntryPoint>>
function main_strings_recursive() {
main();
}
