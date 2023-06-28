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

function test_marking_nested_arrays() :mixed{
  print("\n==============================================\n");
  print("test_marking_nested_arrays():\n");
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
  print_recursive_marking($x, '$x');

  $y = HH\array_mark_legacy_recursive($x);
  print_recursive_marking($y, '$y');
}

function test_unmarking_nested_arrays() :mixed{
  print("\n==============================================\n");
  print("test_unmarking_nested_arrays():\n");
  $c = new C();
  $x = HH\array_mark_legacy(varray[
    vec[$c],
    varray[$c],
    vec[
      HH\array_mark_legacy(darray['c' => $c, 'v' => varray[$c]]),
      vec[$c],
      varray[$c],
    ],
  ]);
  print_recursive_marking($x, '$x');

  $y = HH\array_unmark_legacy_recursive($x);
  print_recursive_marking($y, '$y');
}

<<__EntryPoint>>
function main() :mixed{
  test_marking_nested_arrays();
  test_unmarking_nested_arrays();
}
