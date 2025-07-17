<?hh

function get_vector(string $_): Vector<mixed> {
  return Vector {};
}

function takes_container(Container<mixed> $_): void {}

function my_repro_func(string $tagging_str, bool $test): void {
  $taggings = Vector {};
  if ($test) {
    $taggings = get_vector($tagging_str);
  }

  foreach ($taggings as $res) {
    if ($res is Container<_>) {
      takes_container($res);
    }
  }
}
