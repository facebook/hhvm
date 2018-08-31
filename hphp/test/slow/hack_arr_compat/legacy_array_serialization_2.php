<?hh


// make sure that, e.g. because empty arrays are shared, we don't pollute
// these with the legacy bit
<<__EntryPoint>>
function main_legacy_array_serialization_2() {
echo serialize(dict[]) . "\n";
echo serialize(HH\enable_legacy_behavior(dict[])) . "\n";
echo serialize(dict[]) . "\n";
echo serialize(vec[]) . "\n";
echo serialize(HH\enable_legacy_behavior(vec[])) . "\n";
echo serialize(vec[]) . "\n";
}
