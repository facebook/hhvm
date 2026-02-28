<?hh

class C {}

function print_recursive_marking_helper($x, $path) :mixed{
  if (HH\is_any_array($x)) {
    $marking = HH\is_array_marked_legacy($x) ? "marked" : "unmarked";
    print($path.': '.$marking."\n");
    foreach ($x as $k => $v) {
      print_recursive_marking_helper($v, $path.'['.$k.']');
    }
  } else {
    print($path.": <not an array>\n");
  }
}

function print_recursive_marking($x, $name) :mixed{
  print("\nPrinting marking for ".$name.":\n");
  var_dump($x);
  print_recursive_marking_helper($x, $name);
}

function test_marking_nested_arrays($recursive) :mixed{
  print("\n==============================================\n");
  print('test_marking_nested_arrays($recursive='.str($recursive)."):\n");
  $c = new C();
  $x = vec[
    vec[$c],
    vec[$c],
    vec[
      dict['c' => $c, 'v' => vec[$c]],
      vec[$c],
      vec[$c],
    ],
  ];
  print_recursive_marking($x, '$x');

  $y = HH\array_mark_legacy($x, $recursive);
  print_recursive_marking($y, '$y');
}

// Nested refcount 1 arrays. We should update them in place.
function test_cow_optimizations($recursive) :mixed{
  print("\n==============================================\n");
  print('test_cow_optimizations($recursive='.str($recursive)."):\n");
  $x = HH\array_mark_legacy(vec[vec[new C()]], $recursive);
  print_recursive_marking($x, '$x');
}

// Even though $x['a']['b'] has refcount 1, we should not COW it.
function test_unable_to_cow($recursive) :mixed{
  print("\n==============================================\n");
  print('test_unable_to_cow($recursive='.str($recursive)."):\n");
  $x = dict['a' => dict['b' => vec[new C()]]];
  $y = HH\array_mark_legacy(vec[$x['a']], $recursive);
  print_recursive_marking($x, '$x');
  print_recursive_marking($y, '$y');
}

function str($bool) :mixed{
  return $bool ? 'true' : 'false';
}

<<__EntryPoint>>
function main() :mixed{
  foreach (vec[true, false] as $recursive) {
    test_marking_nested_arrays($recursive);
    test_cow_optimizations($recursive);
    test_unable_to_cow($recursive);
  }
}
