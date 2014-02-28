<?hh

// Test that ImmSets can be cast to bool.

function main() {
  var_dump((bool)(ImmSet {}));
  var_dump((bool)(ImmSet {1, 2, 3}));
}

main();
