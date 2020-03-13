<?hh

class NotARecord {}

function foo(): void {
  $bar = NotARecord['x' => 1];
}
