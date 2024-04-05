<?hh
class JustAClass {
  function isEnabled(): bool {
    return true;
  }
}

final class JustAFinalClass extends JustAClass {
  function isDisabled(): bool {
    return false;
  }
}
