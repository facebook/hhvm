<?hh

// TODO(T125867158): This is a test case capturing the inaccurate inference of
// optional nature of 'a' due to multiple returns.
function multi_return(): dict<string, mixed> {
  $b = true;
  if ($b) {
    return dict['a' => 42];
  }
  return dict['b' => $b];
}
