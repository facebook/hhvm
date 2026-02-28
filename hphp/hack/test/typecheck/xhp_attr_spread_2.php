<?hh
class :baz extends XHPTest {}
class :foo extends XHPTest {
  attribute string bar;
}

function spread(): void {
  // XHP should be allowed to spread
  <foo {...<baz />} bar="bar"></foo>;
}
