<?hh // strict
class :baz {}
class :foo {
  attribute string bar;
}

function spread(): void {
  <foo {...shape('a' => 1)}></foo>;
  <foo {...<baz />} bar="bar"></foo>;
}
