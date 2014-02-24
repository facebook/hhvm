<?hh

// Test that FrozenSets can be cast to bool.

function main() {
  var_dump((bool)(FrozenSet {}));
  var_dump((bool)(FrozenSet {1, 2, 3}));
}

main();
