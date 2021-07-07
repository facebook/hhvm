<?hh

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function set(int $context, (function (): int) $f)[policied] {
    echo 'Setting context to ' . $context . "\n";
    return parent::set($context, $f);
  }
  public static function getContext()[policied]: ?int {
    return parent::get();
  }
}

function addFive()[policied_of] {
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
function main() {
  $result = IntContext::set(5, addFive<>); // FAIL
  var_dump($result);
  $result = HH\Coeffects\enter_policied_of('IntContext', 5, addFive<>);
  var_dump($result);
}
