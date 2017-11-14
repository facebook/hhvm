<?hh // strict

class :foo {
  attribute string bar;
}

function test(): void {
  <foo {...<foo {...<foo bar="bar" {...<foo />} />} />} bar="baz" {...<foo />} />;
}
