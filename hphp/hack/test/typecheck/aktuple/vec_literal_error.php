<?hh //strict

function test(): void {
  $a = varray[4, 'aaa'];

  take_string($a[0]);
}

function take_string(string $_): void {}
