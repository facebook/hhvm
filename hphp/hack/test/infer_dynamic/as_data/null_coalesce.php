<?hh

function test_null_coalesce_string(dynamic $d): void {
  $x = $d['ip'] ?? '';
  takes_string($x);
}

function test_null_coalesce_int(dynamic $d): void {
  $x = $d['count'] ?? 0;
  takes_int($x);
}

function test_null_coalesce_idx(dynamic $d): void {
  $x = idx($d, 'name') ?? 'default';
  takes_string($x);
}

function takes_string(string $_): void {}
function takes_int(int $_): void {}
