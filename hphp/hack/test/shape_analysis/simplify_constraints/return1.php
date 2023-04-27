<?hh

// Report a shape type at the return type hint
function f(): dict<string, mixed> {
  return dict['a' => 42];
}
