<?hh // strict
class :foo extends XHPTest {
  attribute string bar;
}

function spread(): void {
  // Non-XHP is not legal to spread
  <foo bar="baz" {...shape('a' => 1)} />;
}
