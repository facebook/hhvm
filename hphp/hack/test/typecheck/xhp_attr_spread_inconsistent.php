<?hh // strict

class :foo {
  attribute string name;

  public async function genRender(): Awaitable<mixed> {
    // Another case of unsoundness: we pretend that `this` means the class
    // where the spread is written, even though it may be a :subfoo.
    return <bar {...$this} />;
  }
}

class :subfoo extends :foo {
  attribute string age;
}

class :bar extends XHPTest {
  attribute int age;
}
