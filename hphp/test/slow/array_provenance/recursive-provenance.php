<?hh

class C {}

function print_recursive_provenance_helper($x, $path) {
  if (HH\is_any_array($x)) {
    print($path.': '.HH\get_provenance($x)."\n");
    foreach ($x as $k => $v) {
      print_recursive_provenance_helper($v, $path.'['.$k.']');
    }
  } else {
    print($path.": <not an array>\n");
  }
}

function print_recursive_provenance($x, $name) {
  print("\nPrinting provenance for ".$name.":\n");
  var_dump($x);
  print_recursive_provenance_helper($x, $name);
}

function test_recursive_provenance() {
  print("\n==============================================\n");
  print("test_recursive_provenance:\n");
  $c = new C();
  $x = varray[
    vec[$c],
    varray[$c],
    vec[
      darray['c' => $c, 'v' => varray[$c]],
      vec[$c],
      varray[$c],
    ],
  ];
  print_recursive_provenance($x, '$x');

  $y = HH\tag_provenance_here($x);
  print_recursive_provenance($y, '$y');
}

// Nested refcount 1 arrays. We should update them in place.
function test_cow_optimizations() {
  print("\n==============================================\n");
  print("test_cow_optimizations:\n");
  $x = HH\tag_provenance_here(
    varray[varray[new C()]],
  );
  print_recursive_provenance($x, '$x');
}

// Even though $x['a']['b'] has refcount 1, we should not COW it.
function test_unable_to_cow() {
  print("\n==============================================\n");
  print("test_unable_to_cow:\n");
  $x = darray['a' => darray['b' => varray[new C()]]];
  $y = HH\tag_provenance_here(varray[$x['a']]);
  print_recursive_provenance($x, '$x');
  print_recursive_provenance($y, '$y');
}

function test_object_notice() {
  print("\n==============================================\n");
  print("test_object_notice:\n");
  HH\tag_provenance_here(new C());
  HH\tag_provenance_here(new C(), TAG_PROVENANCE_HERE_DONT_WARN_ON_OBJECTS);
}

<<__EntryPoint>>
function main() {
  test_recursive_provenance();
  test_cow_optimizations();
  test_unable_to_cow();
  test_object_notice();
}
