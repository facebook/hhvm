<?hh

// Test that FixedSets can be cast to bool.

function main() {
  var_dump((bool)(FixedSet {}));
  var_dump((bool)(FixedSet {1, 2, 3}));
}

main();
