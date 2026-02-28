<?hh
// Check that gzinflate fails when output would be > limit.
function inflateAboveLimit($original) :mixed{
  $packed=gzdeflate($original);
  for ($i = 1; $i < strlen($original); $i += 1) {
    $unpacked=gzinflate($packed, $i);
    if (!($unpacked === false)) {
      echo "Unexpected success: " . $original . " " . $i . "\n";
    }
  }
}

// Test highly compressible strings.
<<__EntryPoint>>
function main_gzinflate_limit() :mixed{
for ($reps = 2; $reps <= 32; $reps += 1) {
  inflateAboveLimit(str_repeat("a", $reps));
}
// Less compressible...
inflateAboveLimit("incompressible?");
inflateAboveLimit("incompressible (more or less)");
}
