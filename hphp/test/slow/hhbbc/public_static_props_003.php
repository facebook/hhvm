<?hh

class D {
  function go() :mixed{
    static::$boom = new stdClass;
  }
}

class Y extends D {
  public static $boom = 'heh';
}

function go() :mixed{
  var_dump(Y::$boom);
  var_dump(is_string(Y::$boom));
  (new Y)->go();
  var_dump(Y::$boom);
  var_dump(is_string(Y::$boom));
  var_dump(is_object(Y::$boom));
}


<<__EntryPoint>>
function main_public_static_props_003() :mixed{
go();
}
