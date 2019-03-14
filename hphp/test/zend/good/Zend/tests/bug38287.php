<?php
error_reporting(0);

something::do_something();

// $not_there is really NULL
var_dump($not_there);

// error occurs here: execution should never get inside the if condition because $not_there is NULL
if ($not_there["invalid_var"]) {
  // will print NULL (which is ok, but execution should never get here if the value is NULL)
  var_dump($not_there["use_authmodule"]);
  // will print "PATH:Array"
  print "PATH:".$not_there["use_authmodule"]."\n";
}

class something {

  private static $get_objectObject =NULL;
  public static function get_object() {
    if (self::$get_objectObject===NULL)
    self::$get_objectObject=new something;
    return self::$get_objectObject;
  }

  public static function do_something() {
    self::get_object()->vars[]=1;
    self::get_object()->vars[]=2;
    self::get_object()->vars[]=3;
    var_dump(self::get_object()->vars);
  }
}
?>
