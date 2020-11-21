<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class C {
  public static int $x = 42;

  private function m_allowed()[\HH\Capabilities\Globals]: int {
    return static::$x; // ok (context defaults has the Globals capability)
  }
  private function m_banned()[]: int {
    return static::$x; // error
  }
}

function f_allowed()[\HH\Capabilities\Globals]: int {
  $l_disallowed = ()[] ==> C::$x; // error
  return C::$x; // ok
}

function f_banned()[non_det]: int {
  $l_allowed = ()[\HH\Capabilities\Globals] ==> C::$x; // ok
  return C::$x; // error
}
