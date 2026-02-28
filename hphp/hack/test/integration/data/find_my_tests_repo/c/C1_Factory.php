<?hh

class C1_Factory {

  public static function makeSup(): C1_Sup {
    return new C1_Sup();
  }

  public static function makeMid(): C1_Mid {
    return new C1_Mid();
  }

  public static function makeSub(): C1_Sub {
    return new C1_Sub();
  }

}
