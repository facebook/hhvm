<?hh // partial

function my_compact<T>(Vector<?T> $vector): Vector<T> {
  return Vector {};
}

function test(bool $b): ?string {

  $nullable = Vector { null }; // Vector([?T]])
  $non_nullable = my_compact($nullable); // Vector([T])

  if ($b) {
    $res = $nullable;
  } else {
    $res = $non_nullable;
  }
  // integrating if branches unifies ?T with T, resulting in recursive type
  hh_show($res[0]); // Toption(Toption(Toption(Toption(Toption(...)))))
  return $res[0];
}
