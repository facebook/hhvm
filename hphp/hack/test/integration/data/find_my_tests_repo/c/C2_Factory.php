<?hh

class C2_Factory {

  public static function makeSup(): C2_Sup {
    return new C2_Sup();
  }

  public static function makeMid(): C2_Mid {
    return new C2_Mid();
  }

  public static function makeSub(): C2_Sub {
    return new C2_Sub();
  }

}
