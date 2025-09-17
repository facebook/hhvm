<?hh

class B_Uses {

  public static function usesDist1(B_Def $b): void {
    $b->foo();
  }

  public static function usesDist2(B_Def $b): void {
    self::usesDist1($b);
  }

  public static function usesDist3(B_Def $b): void {
    self::usesDist2($b);
  }

}
