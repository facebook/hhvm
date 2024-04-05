<?hh

trait MyTrait {
  function foo(): bool {
    return false;
  }
}

final class JustAClassUsingATrait {
  use MyTrait;
  function isConvoluted(): bool {
    return true;
  }
}
