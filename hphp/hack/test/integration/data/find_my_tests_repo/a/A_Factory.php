<?hh

// This class exists so that our tests don't directly mention the A_* types.
// Instead, they call this factory.
//
// We avoid actually constructing these objects to avoid spurious references.
class A_Factory {

  public static function makeSub(): A_Sub {
    return null as nonnull as A_Sub;
  }

  public static function makeMiddle(): A_Middle {
    return null as nonnull as A_Middle;
  }

  public static function makeSup(): A_Super {
    return null as nonnull as A_Super;
  }

  public static function makeSibling(): A_Sibling {
    return null as nonnull as A_Sibling;
  }

  public static function makeSubNoOverride(): A_SubNoOverride {
    return null as nonnull as A_SubNoOverride;
  }

}
