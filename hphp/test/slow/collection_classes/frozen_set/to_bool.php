<?hh

// Test that ImmSets can be cast to bool.

function main() :mixed{
  var_dump((bool)(ImmSet {}));
  var_dump((bool)(ImmSet {1, 2, 3}));
}


<<__EntryPoint>>
function main_to_bool() :mixed{
main();
}
