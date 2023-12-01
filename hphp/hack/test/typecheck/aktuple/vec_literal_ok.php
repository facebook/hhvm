<?hh

function test(): void {
  $a = vec[4, 'aaa'];

  take_int($a[0]);
  take_string($a[1]);
}

function take_int(int $_): void {}
function take_string(string $_): void {}
