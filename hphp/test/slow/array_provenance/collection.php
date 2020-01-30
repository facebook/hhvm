<?hh

// A collection object should not have provenance, but the Hack array
// that it wraps should have provenance. We display both for testing.
<<__ProvenanceSkipFrame>>
function show_provenance($x) {
  var_dump(dict[
    'outer' => HH\get_provenance($x),
    'inner' => HH\get_provenance(dict($x)),
  ]);
}

<<__ProvenanceSkipFrame>>
function test_collection_explicit_tag() {
  $test = Set { 'test' };
  return HH\tag_provenance_here($test);
}

<<__ProvenanceSkipFrame>>
function test_collection_literal() {
  return Set { 'test' };
}

<<__EntryPoint>>
function main() {
  show_provenance(test_collection_explicit_tag());
  show_provenance(test_collection_literal());
  throw new Error('done');
}
