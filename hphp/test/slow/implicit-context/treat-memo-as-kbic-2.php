<?hh

class Base implements HH\IPureMemoizeParam {
  public function getInstanceKey()[]: string {
    return 'KEY' . $this->name();
  }
  public function name()[]: string { return static::class; }
}

abstract final class ClassContext extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = Base;
  const ctx CRun = [defaults];
  public static function start(this::TData $context, (function (): int) $f)[this::CRun, ctx $f] {
    return parent::runWith($context, $f);
  }
  public static function getContext()[this::CRun]: this::TData {
    return parent::get();
  }
  public static function exists()[this::CRun]: bool {
    return parent::exists() as bool;
  }
}

class A extends Base {}

class B extends Base {
  <<__Memoize(#KeyedByIC)>>
  public function memo_kbic($a, $b)[defaults]: mixed {
    $context = (ClassContext::getContext()?->name() ?? 'null');
    echo "args: $a, $b name: $context\n";
  }

  <<__Memoize(#MakeICInaccessible)>>
  public function memo_inaccessible($a, $b)[defaults]: mixed {
    $context = (ClassContext::getContext()?->name() ?? 'null');
    echo "args: $a, $b name: $context\n";
  }

  <<__Memoize>>
  public function memo_default($a, $b)[defaults]: mixed {
    $context = (ClassContext::getContext()?->name() ?? 'null');
    echo "args: $a, $b name: $context\n";
  }

}


<<__Memoize(#KeyedByIC), __DynamicallyCallable>>
function memo_kbic($a, $b)[defaults]: mixed{
  $context = (ClassContext::getContext()?->name() ?? 'null');
  echo "args: $a, $b name: $context\n";
}

<<__Memoize(#MakeICInaccessible), __DynamicallyCallable>>
function memo_inaccessible($a, $b)[defaults]: mixed{
  $context = (ClassContext::getContext()?->name() ?? 'null');
  echo "args: $a, $b name: $context\n";
}

<<__Memoize, __DynamicallyCallable>>
function memo_default($a, $b)[defaults]: mixed{
  $context = (ClassContext::getContext()?->name() ?? 'null');
  echo "args: $a, $b name: $context\n";
}

function f()[defaults]: mixed{
  $klass_b = new B(0);
  $tryout = function($memo_function, $a, $b) use ($klass_b) {
    try {
      HH\dynamic_fun($memo_function)($a, $b);
    } catch (Exception $e) {
      echo "Function $memo_function throws: ".$e->getMessage() . "\n";
    }

    try {
      $klass_b->$memo_function($a, $b);
    } catch (Exception $e) {
      echo "Method B->$memo_function throws: ".$e->getMessage() . "\n";

    }
  };
  $tryout('memo_kbic', 1, 2);
  $tryout('memo_inaccessible', 3, 4);
  $tryout('memo_default', 5, 6);
}


<<__EntryPoint>>
function main(): mixed{
  ClassContext::start(new A(0), f<>);
}
