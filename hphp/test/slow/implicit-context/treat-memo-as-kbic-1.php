<?hh

class Base implements HH\IMemoizeParam {
  public function getInstanceKey()[]: string {
    return 'KEY' . $this->name();
  }
  public function name()[]: string { return static::class; }
}

abstract final class ClassContext extends HH\ImplicitContext {
  const type T = Base;
  const bool IS_MEMO_SENSITIVE = true;
  const ctx CRun = [defaults];
  public static function start(Base $context, (function (): int) $f)[this::CRun, ctx $f] {
    return parent::runWith($context, $f);
  }
  public static function getContext()[this::CRun]: Base {
    return parent::get() as nonnull;
  }
  public static function exists()[this::CRun]: bool {
    return parent::exists() as bool;
  }
}

class A extends Base {}

class B extends Base {
  <<__Memoize(#KeyedByIC)>>
  public function memo_kbic($a, $b)[defaults]: mixed {
    $context = ClassContext::getContext()->name();
    echo "args: $a, $b name: $context\n";
  }

  <<__Memoize(#MakeICInaccessible)>>
  public function memo_inaccessible($a, $b)[defaults]: mixed {
    $context = ClassContext::getContext()->name();
    echo "args: $a, $b name: $context\n";
  }

  <<__Memoize>>
  public function memo_default($a, $b)[defaults]: mixed {
    $context = ClassContext::getContext()->name();
    echo "args: $a, $b name: $context\n";
  }

}


<<__Memoize(#KeyedByIC)>>
function memo_kbic($a, $b)[defaults]: mixed{
  $context = ClassContext::getContext()->name();
  echo "args: $a, $b name: $context\n";
}

<<__Memoize(#MakeICInaccessible)>>
function memo_inaccessible($a, $b)[defaults]: mixed{
  $context = ClassContext::getContext()->name();
  echo "args: $a, $b name: $context\n";
}

<<__Memoize>>
function memo_default($a, $b)[defaults]: mixed{
  $context = ClassContext::getContext()->name();
  echo "args: $a, $b name: $context\n";
}

function f()[defaults]: mixed{
  $klass_b = new B;
  $tryout = function($memo_function, $a, $b) use ($klass_b) {
    try {
      $memo_function($a, $b);
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
  ClassContext::start(new A, f<>);
}
