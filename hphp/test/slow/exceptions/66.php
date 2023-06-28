<?hh

class C {
  <<__NEVER_INLINE>>
  function g() :mixed{
    $ex = new Exception();
    $bt = $ex->getTrace();
    foreach ($bt as $k => $_) {
      $frame = $bt[$k];
      unset($frame['file']);
      unset($frame['line']);
      unset($frame['args']);
      ksort(inout $frame);
      $bt[$k] = $frame;
    }
    var_dump($bt);
  }
  <<__NEVER_INLINE>>
  function f() :mixed{
    $this->g();
  }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->f();
echo "------------------------\n";
Exception::setTraceOptions(DEBUG_BACKTRACE_PROVIDE_OBJECT);
$obj->f();
}
