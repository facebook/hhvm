<?hh

function getUntyped() /* : TAny */ {
  return 'test';
}

function getEmptyShape(): shape() {
  return shape(
    'test' => getUntyped(),
  );
}
