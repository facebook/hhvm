<?hh // strict

class R_Base {
  public static function bad(): void {
    print "bad\n";
  }

  public static function good3(): void {
    print "good3\n";
  }
}

class R_Derived extends R_Base {}

trait R_ComponentTrait {
  public static function good2(): void {
    print "good2\n";
  }
}

trait R_Trait {
  require extends R_Base;

  use R_ComponentTrait;

  public static function good(): void {
    print "good\n";
  }
}

function test(): void {
  // Calling something directly on the trait should pass.
  R_Trait::good();

  // Calling something brought in from trait composition should pass.
  R_Trait::good2();

  // Make sure we don't accidentally flag class inheritance.
  R_Derived::good3();

  // This should fail since it's not truly defined.
  R_Trait::bad();
}
