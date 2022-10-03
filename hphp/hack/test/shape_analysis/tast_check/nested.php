<?hh

function f(): dict<string, mixed> {
  // We should get no results for because both dictinaries escape their local
  // definition.
  return dict['a' => dict['b' => 42]];
}
