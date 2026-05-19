<?hh

function takes_int(int $_): void {}
function takes_string(string $_): void {}

function test_try_catch(dynamic $d): void {
  try {
    takes_int($d);
  } catch (Exception $_) {
    takes_string($d);
  }
}
