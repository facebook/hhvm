<?hh

class something {

  private static $get_objectObject = NULL;
  public static function get_object() :mixed{
    if (self::$get_objectObject === NULL)
      self::$get_objectObject = new something;
    return self::$get_objectObject;
  }

  public static function do_something() :mixed{
    self::get_object()->vars = vec[];
    self::get_object()->vars[] = 1;
    self::get_object()->vars[] = 2;
    self::get_object()->vars[] = 3;
    var_dump(self::get_object()->vars);
  }
}

<<__EntryPoint>>
function main(): void {
  error_reporting(0);

  something::do_something();

  try {
    // $not_there is really NULL
    var_dump($not_there);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
