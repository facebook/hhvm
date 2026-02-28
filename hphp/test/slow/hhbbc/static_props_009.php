<?hh

class y {}
class x {
  private static $set = 12;
  private static $hm = "asd";
  private static $k;
  public static function go() :mixed{
    self::$k = new y;
    for ($i = 0; $i < 10; ++$i) {
      self::$set = $i; // enough to fool hphpc into leaving the
                       // IssetS,EmptyS opcodes
      var_dump(!(self::$set ?? false));
      var_dump(isset(self::$set));
    }
  }
}


<<__EntryPoint>>
function main_static_props_009() :mixed{
x::go();
}
