<?hh

function test(mixed $fields): void {
  $_ = shape(...$fields);
}
