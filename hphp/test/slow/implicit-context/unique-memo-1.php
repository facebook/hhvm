<?hh

<<__Memoize(#KeyedByIC)>>
function memo()[zoned] :mixed{
  echo "memo called\n";
}

function f()[zoned] :mixed{
  $key = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
  $str_hash = HH\Lib\Str\join($key, ', '); // can't do var_dump due to keyedByIC
  echo $str_hash . "\n";
  memo();
}

class MemoSensitiveData implements HH\IPureMemoizeParam {
  public int $p;
  public function __construct(int $p) { $this->p = $p; }
  public function getPayload()[]: int {
    return $this->p;
  }
  public function getInstanceKey()[]: string {
    return 'strkey'.$this->getPayload();
  }
}

trait T {
  const type TData = MemoSensitiveData;
  const ctx CRun = [zoned];
  public static function set($value, $fun) :mixed{ parent::runWith($value, $fun); }
}

final class IntContext extends HH\HHVMTestMemoSensitiveImplicitContext { use T; }
final class IntContext1 extends HH\HHVMTestMemoSensitiveImplicitContext { use T; }

<<__EntryPoint>>
function main() :mixed{
  IntContext::set(new MemoSensitiveData(11), f<>);
  IntContext1::set(new MemoSensitiveData(1), f<>);
}
