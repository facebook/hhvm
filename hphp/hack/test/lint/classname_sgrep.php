<?hh

function test($y, $fd) {
  Classes::get('clazz', $y);
  $fd->enumeration(A, B, 'clazz', $y);
  new EnumParam('clazz');
  new EnumValueParam('clazz');
}
