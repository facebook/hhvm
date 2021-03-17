<?hh

class X {
  <<__NEVER_INLINE>>
  function __construct() {
  }
}

function handler($kind, $name) {
  if ($kind == 'exit' && $name == 'X::__construct') throw new Exception;
}

<<__EntryPoint>> function test(): void {
  fb_setprofile(handler<>);
  try {
    new X;
  } catch (Exception $e) {
    echo "ok\n";
  }
}
