<?hh

function test_array_write(dynamic $d): void {
  $d['key'] = 42;
}

function test_array_append(dynamic $d): void {
  $d[] = "hello";
}
