<?hh // strict
class :foo {
  attribute string bar;
}

function spread(): void {
  <foo {...shape('a' => 1)} />;
  <foo {...shape('a' => 1)} bar="baz" />;
  <foo {...shape('a' => 1)} bar="baz" {...false} />;
}
