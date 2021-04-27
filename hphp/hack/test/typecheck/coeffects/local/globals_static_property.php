<?hh

class C {
  public static int $x = 42;

  private function m_allowed()[\HH\Capabilities\AccessGlobals]: int {
    return static::$x; // ok (context defaults has the AccessGlobals capability)
  }
  private function m_banned()[]: int {
    return static::$x; // error
  }
}

function f_allowed()[\HH\Capabilities\AccessGlobals]: int {
  $l_disallowed = ()[] ==> C::$x; // error
  return C::$x; // ok
}

function f_banned()[write_props]: int {
  $l_allowed = ()[\HH\Capabilities\AccessGlobals] ==> C::$x; // ok
  return C::$x; // error
}
