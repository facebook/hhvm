<?hh

class C {
  public static int $x = 42;

  private function m_allowed()[\HH\Capabilities\AccessStaticVariable]: int {
    return static::$x; // ok (context defaults has the AccessStaticVariable capability)
  }
  private function m_banned()[]: int {
    return static::$x; // error
  }
}

function f_allowed()[\HH\Capabilities\AccessStaticVariable]: int {
  $l_disallowed = ()[] ==> C::$x; // error
  return C::$x; // ok
}

function f_banned()[local]: int {
  $l_allowed = ()[\HH\Capabilities\AccessStaticVariable] ==> C::$x; // ok
  return C::$x; // error
}
