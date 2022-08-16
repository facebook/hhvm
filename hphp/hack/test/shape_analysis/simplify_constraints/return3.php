<?hh



function multi_return(): dict<string, mixed> {
  $b = true;
  if ($b) {
    return dict['a' => 42];
  }
  return dict['b' => $b];
}
