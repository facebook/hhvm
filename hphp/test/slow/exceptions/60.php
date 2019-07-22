<?hh

class a extends Exception {
}
class b extends a {
  function dump() {
    echo 'c:', $this->code, '
m:', $this->message, '
';
    echo 'x:', $this->x, '
y:', $this->y, '
';
  }
}

<<__EntryPoint>>
function main_60() {
  if (__hhvm_intrinsics\launder_value(0)) {
    include '60.inc';
  }
  try {
    throw(new b(1, 2));
  } catch (b $e) {
    $e->dump();
  }
}
