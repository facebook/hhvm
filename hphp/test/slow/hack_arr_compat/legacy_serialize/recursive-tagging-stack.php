<?hh

class C {}

// Returns a nested varray that is *approximately* $depth deep.
//
// It will have multiple arrays at the given depth (but we should only warn
// once about blowing the recursion stack for that depth).
function make_recursive_varray($depth) :mixed{
  $result = varray[
    varray[
      varray[varray[new C()], varray[new C()]],
      varray[varray[new C()], varray[new C()]],
    ],
    varray[
      varray[varray[new C()], varray[new C()]],
      varray[varray[new C()], varray[new C()]],
    ],
  ];

  for ($i = 0; $i < $depth - 2; $i++) {
    $result = varray[$result];
  }
  return $result;
}

<<__EntryPoint>>
function main() :mixed{
  $depth = 512;
  $v = HH\array_mark_legacy(make_recursive_varray($depth), true);
  for ($i = 0; $i < $depth + 2; $i++) {
    if (!HH\is_array_marked_legacy($v)) {
      print('First unmarked array at depth '.$depth.".\n");
      break;
    }
    $v = $v[0];
  }
}
