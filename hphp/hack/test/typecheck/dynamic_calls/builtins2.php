<?hh // strict

function foo(): void {}

function get_dynamic(): void {
  $foo_dynamic = HH\dynamic_fun('foo');
  $foo_more_dynamic = HH\dynamic_fun($foo_dynamic);
}
