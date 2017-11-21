<?hh // strict
class :baz {}
class :foo {
  attribute string bar;
}

function spread(): void {
  // XHP should be allowed to spread
  <foo {...<baz />} bar="bar"></foo>;
}
