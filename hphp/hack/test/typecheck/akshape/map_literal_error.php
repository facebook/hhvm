<?hh //strict

function test(): void {
  $a = array('k1' => 4, 'k2' => 'aaa');

  take_string($a['k1']);
}

function take_string(string $_): void {}
