<?hh

function nocase($a, $b) :mixed{
  $la = strtolower((string)$a);
  $lb = strtolower((string)$b);
  return ($la === $lb) ? 0 : (($la > $lb) ? 1 : (-1));
}

function vowel($c) :mixed{
  $k = keyset['a', 'e', 'i', 'o', 'u'];
  return isset($k[$c]);
}

function concat($s) :mixed{
  return $s.$s;
}

function with_keyset($k1) :mixed{
  echo "---- running " . __FUNCTION__ . " with\n";
  var_dump($k1);
  $k2 = keyset['q', 'n'];
  $k3 = keyset['q', 'N'];
  echo "array_diff_assoc: ";
  var_dump(array_diff_assoc($k1, $k2));
  echo "array_diff: ";
  var_dump(array_diff($k1, $k2));
  echo "array_diff_key: ";
  var_dump(array_diff_key($k1, $k3));
  echo "array_diff_uassoc: ";
  var_dump(array_diff_uassoc($k1, $k3, nocase<>));
  echo "array_diff_ukey: ";
  var_dump(array_diff_ukey($k1, $k3, nocase<>));
  echo "array_udiff: ";
  var_dump(array_udiff($k1, $k3, nocase<>));
  echo "array_udiff_assoc: ";
  var_dump(array_udiff_assoc($k1, $k3, nocase<>));
  echo "array_udiff_uassoc: ";
  var_dump(array_udiff_uassoc($k1, $k3, nocase<>, nocase<>));
  echo "array_combine 1: ";
  var_dump(array_combine($k1, keyset[1, 2]));
  echo "array_combine 2: ";
  var_dump(array_combine($k1, keyset[1, 2, 3, 4, 5, 6]));
  echo "array_uintersect_assoc: ";
  var_dump(array_uintersect_assoc($k1, $k3, nocase<>));
  echo "array_uintersect_uassoc: ";
  var_dump(array_uintersect_uassoc($k1, $k3, nocase<>, nocase<>));
  echo "array_intersect_assoc: ";
  var_dump(array_intersect_assoc($k1, $k2));
  echo "array_intersect_key: ";
  var_dump(array_intersect_key($k1, $k2));
  echo "array_intersect_uassoc: ";
  var_dump(array_intersect_uassoc($k1, $k3, nocase<>));
  echo "array_intersect_ukey: ";
  var_dump(array_intersect_ukey($k1, $k3, nocase<>));
  echo "array_intersect: ";
  var_dump(array_intersect($k1, $k2));
  echo "array_uintersect: ";
  var_dump(array_uintersect($k1, $k3, nocase<>));
  echo "array_filter: ";
  var_dump(array_filter($k1, vowel<>));
  echo "array_flip: ";
  var_dump(array_flip($k1));
  echo "array_map: ";
  var_dump(array_map(concat<>, keyset['H', 'A', 'L']));
  echo "array_merge: ";
  var_dump(array_merge(keyset[1, 2, 3], keyset['a', 'b', 'c']));
  echo "array_reverse: ";
  var_dump(array_reverse($k1));
  echo "array_slice: ";
  var_dump(array_slice($k1, 2));

  // The functions below must accept hack arrays as arguments and work
  // like if an array were passed.
  //
  echo "array_key_exists: ";
  var_dump(vec[array_key_exists('x', $k1), array_key_exists('q', $k1)]);
  echo "array_keys: ";
  var_dump(array_keys($k1));
  $k = $k1;
  echo "array_pop: ";
  var_dump(array_pop(inout $k));
  var_dump($k);
  echo "array_product: ";
  var_dump(array_product($k1));
  srand(0);                                        // Determinize that!
  echo "array_rand: ";
  var_dump(array_rand($k1));
  echo "array_reduce: ";
  var_dump(array_reduce($k1, function($s, $x) { return $s . $x; }, ""));
  echo "array_search 1: ";
  var_dump(array_search('n', $k1));
  echo "array_search 2: ";
  var_dump(array_search('x', $k1));
  echo "array_shift: ";
  var_dump(array_shift(inout $k));
  var_dump($k);
  echo "array_sum: ";
  var_dump(array_sum($k1));
  echo "in_array: ";
  var_dump(vec[in_array('x', $k1), in_array('q', $k1)]);
  // list() could be tested here, but it's just weird with keysets
  echo "count: ";
  var_dump(count($k1));
  echo "array_pad 1: ";
  var_dump(array_pad($k1, 7, 'n'));
  echo "array_pad 2: ";
  var_dump(array_pad($k1, 2, 'x'));
  echo "array_replace: ";
  var_dump(array_replace($k1, dict['q' => 'r']));
  echo "array_unique 1: ";
  var_dump(array_unique($k1));
  echo "array_unique 2: ";
  var_dump(array_unique(keyset[]));

  // Those two must work with keysets and preserve the type.
  //
  var_dump(array_push(inout $k, 'i'));
  var_dump(array_unshift(inout $k, 'q'));
  var_dump($k);

  echo "array_splice: ";
  var_dump(array_splice(inout $k, 2));
  var_dump($k);

}
<<__EntryPoint>> function main(): void {
// There must be APC support for keysets (and dicts, vecs).
//
echo "apc_add: ";
var_dump(apc_add('foo', keyset[1, 2, 3]));
echo "apc_fetch: ";
var_dump(__hhvm_intrinsics\apc_fetch_no_check('foo'));

// All the functions below must see keysets (and dicts, vecs)
// as arrays and return an array.
//
echo "array_change_key_case: ";
var_dump(array_change_key_case(keyset["FOO", "Bar"]));
echo "array_chunk 1: ";
var_dump(array_chunk(keyset[1, 2, 3], 2));
echo "array_chunk 2: ";
var_dump(array_chunk(keyset[1, 2, 3], 2, true /* preserve keys */));
echo "array_column 1: ";
var_dump(array_column(
  dict[
    'a' => dict['foo' => 'bar1', 'baz' => 'qux1'],
    'b' => keyset['foo', 'baz'],
  ],
  'foo',
));
echo "array_column 2: ";
var_dump(array_column(
  dict[
    'a' => dict['foo' => 'bar1', 'baz' => 'qux1'],
    'b' => keyset['foo', 'baz'],
  ],
  'foo',
  'baz',
));

with_keyset(keyset['q', 'u', 'e', 'n', 't', 'i', 'n']);
with_keyset(keyset[5, 7, 8, 10]);
with_keyset(keyset["5", "7", "8", "10"]);
with_keyset(keyset[]);

// Recursive functions will convert the keysets they process in a "lazy"
// fashion...
///
$ar1 = dict["colors" => keyset["green", "red"], 0 => 5];
$ar2 = dict[0 => 10, "colors" => keyset["green", "blue"]];
echo "array_merge_recursive: ";
var_dump(array_merge_recursive($ar1, $ar2));
echo "array_replace_recursive 1: "; // The keyset remains in the return value
var_dump(array_replace_recursive($ar1, dict["green" => "blue"]));
echo "array_replace_recursive 2: ";
var_dump(array_replace_recursive($ar1, dict["colors" => dict["green" => "blue"]]));

// These functions should return false or null and emit a warning when passed a
// hack array.
//
echo "array_multisort: ";
$k1 = null;
var_dump(array_multisort1(inout $k1));

// Those should simply return a php array.
//
echo "array_fill_keys: ";
var_dump(array_fill_keys(keyset['fizz', 'buzz'], 42));
echo "array_fill: ";
var_dump(array_fill(10, 2, 42));

// Sorting functions.
//
// TODO
}
