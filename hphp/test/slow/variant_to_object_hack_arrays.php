<?hh


// The important thing here is that we call Variant::toObject on a hack array.
// Extensions use Variant to accept arguments from user code, so all the repro
// recipies below are using that. The extensions themselves are not to blame.

<<__EntryPoint>>
function main_variant_to_object_hack_arrays() :mixed{
$hackArrays = vec[
  vec[],
  dict[],
  keyset[],
];

echo "---- using DateTime::diff\n";
foreach ($hackArrays as $hackArray) {
  @var_dump((new DateTime())->diff($hackArray));
}

echo "---- using DOMXPath::__construct\n";
foreach ($hackArrays as $hackArray) {
  try {
    var_dump(new DOMXPath($hackArray));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
}
