<?hh
abstract class JustAnAbstractClass {
  abstract public static function genCreate(): Awaitable<this>;
  function isEnabled(): bool {
    return true;
  }
}
