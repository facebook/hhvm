<?hh

// Since finale class cannot be overridden, that also means their type constants
// cannot be overridden. This means we do not need to generate expression
// dependent types for these classes
final class IAmFinal {
  const type T = int;

  public static function test(this::T $t): this::T {
    hh_show($t);
    hh_show(new IAmFinal());

    // any int will work since this::T won't be expression dependent
    return 101;
  }
}

function test(IAmFinal $final): void {
  $final::test(101);
}
