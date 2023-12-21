<?hh

class C {}

<<__EntryPoint>>
function main() :mixed{
  // We construct a C so that some of the arrays here are non-static.
  $c = new C();
  $values = dict[
    'vec[]' => vec[],
    'vec[$c]' => vec[$c],
    'HH\\array_mark_legacy(vec[])' => HH\array_mark_legacy(vec[]),
    'HH\\array_mark_legacy(vec[$c])' => HH\array_mark_legacy(vec[$c]),
  ];
  foreach ($values as $k1 => $v1) {
    print("----------------------------------------------------------\n");
    foreach ($values as $k2 => $v2) {
      $result = $v1 === $v2 ? '===' : '!==';
      print("$k1 $result $k2\n");
    }
  }
}
