<?hh

abstract final class IntContext extends HH\ImplicitContext {
  const type T = int;
  public static function set(int $context, (function (): int) $f)[zoned] :mixed{
    echo 'Setting context to ' . $context . "\n";
    return parent::runWith($context, $f);
  }
  public static function getContext()[zoned]: ?int {
    return parent::get();
  }
}

function g()[defaults] :mixed{
  $context = IntContext::getContext() ?? 'null';
  echo "in g context is $context \n";
}

function h_exception()[defaults] :mixed{
  $context = IntContext::getContext() ?? 'null';
  echo "in h_exception context is $context \n";
  echo "throwing exception from h_exception()\n";
  throw new Exception();
}

function f()[zoned] :mixed{
  $context = IntContext::getContext() ?? 'null';
  echo "in f context is $context \n";
  HH\Coeffects\backdoor(g<>);
  $context = IntContext::getContext() ?? 'null';
  echo "back in f context is $context \n";
  try { HH\Coeffects\backdoor(h_exception<>); } catch (Exception $_) {}
  $context = IntContext::getContext() ?? 'null';
  echo "back in f context is $context \n";
}

<<__EntryPoint>>
function main() :mixed{
  HH\Coeffects\enter_zoned_with('IntContext', 5, f<>);
}
