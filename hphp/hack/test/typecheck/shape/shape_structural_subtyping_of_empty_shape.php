<?hh // partial

function getUntyped() {
  return 'test';
}

function getEmptyShape(): shape() {
  return shape(
    'test' => getUntyped(),
  );
}
