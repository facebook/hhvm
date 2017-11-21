<?hh // strict

class :foo {
  attribute string bar;
}

function test(): void {
  // Heavily nested XHP spreads are legal as well
  <foo {...<foo {...<foo bar="bar" {...<foo />} />} />} bar="baz" {...<foo />} />;
}
