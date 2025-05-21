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
  public arraykey $p;
  public function __construct(arraykey $p) { $this->p = $p; }
  public function getInstanceKey()[]: string {
    return $this->p;
  }
}

final class ArraykeyContext extends HH\HHVMTestMemoSensitiveImplicitContext {
  const type TData = MemoSensitiveData;
  const ctx CRun = [zoned];
  public static function set($value, $fun) :mixed{ parent::runWith($value, $fun); }
}


<<__EntryPoint>>
function main() :mixed{
  ArraykeyContext::set(new MemoSensitiveData("123"), f<>);
}
