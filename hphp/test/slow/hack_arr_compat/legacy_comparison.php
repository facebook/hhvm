<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
  // We construct a C so that some of the arrays here are non-static.
  $c = new C();
  $values = dict[
    'vec[]' => vec[],
    'vec[$c]' => vec[$c],
    'varray[]' => varray[],
    'varray[$c]' => varray[$c],
    'HH\\array_mark_legacy(varray[])' => HH\array_mark_legacy(varray[]),
    'HH\\array_mark_legacy(varray[$c])' => HH\array_mark_legacy(varray[$c]),
  ];
  foreach ($values as $k1 => $v1) {
    print("----------------------------------------------------------\n");
    foreach ($values as $k2 => $v2) {
      $result = $v1 === $v2 ? '===' : '!==';
      print("$k1 $result $k2\n");
    }
  }
}
