<?hh
final class X {
  public readonly function getValue(): readonly int {
    return readonly 5;
  }
}

final class XContainer {
  public function __construct(private readonly int $value) {}
}

abstract final class AA {

  public static function makeXContainerExample(): readonly XContainer {
    $x = new X();
    return readonly new XContainer(
      $x->getValue(),
    );
  }
}
