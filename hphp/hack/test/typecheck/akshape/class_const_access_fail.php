<?hh

/**
 * This should be an error, but it's broken for shapes too...
 */
class C {
  const KEY = 'key';
  const SAME_KEY = 'key';
}

function test(): void {
  $s = array();

  $s[C::KEY] = 4;
  $s[C::SAME_KEY] = 'aaa';

  take_int($s[C::KEY]);
}

function take_int(int $_) {}
