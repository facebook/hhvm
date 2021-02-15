<?hh // strict

class :foo extends XHPTest {
  attribute enum {'herp', 'derp'} bar;
}

function main(): void {
  <foo bar="herp" />; // no error
  $derp = 'derp';
  <foo bar={$derp} />; // no error
}
