<?hh

class C {
  public static function cfunc($x) {
    return $x * 2;
  }

  public static function postSend() {
    \var_dump("postSend");
  }
  public static function onShutdown() {
    \var_dump("shutdown");
  }
}

function afunc($x) {
    return $x * 3;
}
