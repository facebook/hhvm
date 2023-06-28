<?hh

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function set(int $context, (function (): int) $f)[zoned, ctx $f] :mixed{
    echo 'Setting context to ' . $context . "\n";
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: ?int {
    return parent::get();
  }
}

function addFive()[zoned_with] :mixed{
  return IntContext::getContext() + 5;
}

<<__EntryPoint>>
function main() :mixed{
  $result = IntContext::set(5, addFive<>); // FAIL
  var_dump($result);
  $result = HH\Coeffects\enter_zoned_with('IntContext', 5, addFive<>);
  var_dump($result);
}
