<?hh

function print_serialize($arr) {
  echo serialize($arr) . "\n";
}

function print_legacy($arr) {
  print_serialize(HH\enable_legacy_behavior($arr));
}

function test_cow_legacy_bit($arr) {
  print_legacy($arr);
  print_serialize($arr);
}


// test some crazy conversions
<<__EntryPoint>>
function main_legacy_array_serialization_3() {
echo "=== test conversions ===\n";
print_serialize(dict(HH\enable_legacy_behavior(vec[1, 2, 3])));
print_serialize(vec(HH\enable_legacy_behavior(dict["foo" => "bar"])));
print_serialize(dict(darray(HH\enable_legacy_behavior(dict["foo" => "bar"]))));
print_serialize(vec(varray(HH\enable_legacy_behavior(vec[1, 2, 3]))));

echo "=== test copy on set legacy bit ===\n";
test_cow_legacy_bit(vec[1, 2, 3]);
echo "\n";
test_cow_legacy_bit(dict["foo" => "bar"]);

echo "=== test legacy bit preserved on copy ===\n";
$arr = HH\enable_legacy_behavior(vec[1, 2, 3]);
$arr2 = $arr;
$arr[] = 42;
print_serialize($arr);
print_serialize($arr2);

echo "\n";

$arr = HH\enable_legacy_behavior(dict["foo" => "bar"]);
$arr2 = $arr;
$arr2["baz"] = 4;
print_serialize($arr);
print_serialize($arr2);
}
