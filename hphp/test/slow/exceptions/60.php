<?hh

class a extends Exception {
}
class b extends a {
  function dump() :mixed{
    echo
      'c:',
      $this->code,
      '
m:',
      $this->message,
      '
';
  }
}

<<__EntryPoint>>
function main_60() :mixed{
  if (__hhvm_intrinsics\launder_value(0)) {
    include '60.inc';
  }
  try {
    throw (new b('1', 2));
  } catch (b $e) {
    $e->dump();
  }
}
