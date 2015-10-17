<?hh //strict

function take_string(string $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = array('int' => 4);
  } else {
    $a = array('string' => 'aaa');
  }

  take_string($a['int']);
}
