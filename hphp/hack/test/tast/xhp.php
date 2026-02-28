<?hh

class :foo extends XHPTest implements XHPChild {
  attribute enum {'herp', 'derp'} bar;
}

function main(): void {
  // Attribute list tests.
  <foo bar="herp" />;
  $derp = 'derp';
  <foo bar={$derp} />;

  // Element list tests.
  <foo bar="herp">
    <foo bar="derp">
      <foo bar="herp" />
    </foo>
  </foo>;
}
