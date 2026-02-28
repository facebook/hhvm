<?hh

<<__EntryPoint>>
function main(): void {
  try {
    try {
      throw new Exception('impure exception');
    } catch (Exception $e) {
      throw new LogicException('pure exception', 0, $e);
    }
  } catch (Exception $e) {
    var_dump(Exception::toStringPure($e));
    echo "===\n";
    var_dump(Exception::toStringPure($e, ($_)[] ==> 'my placeholder'));
  }
}
