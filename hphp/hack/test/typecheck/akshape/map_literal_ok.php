<?hh //strict

function test(): void {
  $a = array('k1' => 4, 'k2' => 'aaa');

  take_int($a['k1']);
  take_string($a['k2']);
}

function take_int(int $_): void {}
function take_string(string $_): void {}
