<?hh
// Copyright 2004-2015 Facebook. All Rights Reserved.

class c {
  private static $thing;
  private static $otherthing;

  public static function doit($id, $value) :mixed{
    self::$thing = dict[];
    self::$thing[(string)$id] = $value;
    self::$otherthing = dict[];
    self::$otherthing[(string)$id] = $value;
  }

  public static function dump() :mixed{
    var_dump(self::$thing, self::$otherthing);
  }
}

function main() :mixed{
  c::doit(0, 'hello');
  c::dump();
}
<<__EntryPoint>> function main_entry(): void {
echo "Calling main\n";
main();
echo "Done\n";
}
