<?hh

class C<reify Ta, reify Tb> {
  function g<reify T1>() :mixed{
    try {
      throw new Exception();
    } catch (Exception $e) {
      echo $e->getTraceAsString() . "\n";
    }
  }
  <<__NEVER_INLINE>>
  function f<reify T1>() :mixed{
    $this->g<T1>();
  }
}
<<__EntryPoint>>
function entrypoint_backtrace(): void {

  $c = new C<int, shape('a' => int, 'b' => string)>();
  $c->f<string>();
}
