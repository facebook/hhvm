<?hh

function takes_int(int $_): void {}
function takes_string(string $_): void {}
function get_dynamic(): dynamic { return 0 as dynamic; }

function test_reassign(): void {
  $x = get_dynamic();
  takes_int($x);
  $x = get_dynamic();
  takes_string($x);
}
