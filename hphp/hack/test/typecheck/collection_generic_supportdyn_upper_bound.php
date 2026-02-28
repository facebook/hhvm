<?hh

function takes_container(Container<mixed> $_): void {}

function mymain(bool $test, Vector<Container<mixed>> $vcmixed): void {
  $v = Vector {};
  $v_alias = $v;
  if ($test) {
    $v = $vcmixed;
  }
  $v_alias[] = null; // comment this out and there's no error
  foreach ($v as $item) {
    if ($item is Container<_>) {
      takes_container($item);
    }
  }
}
