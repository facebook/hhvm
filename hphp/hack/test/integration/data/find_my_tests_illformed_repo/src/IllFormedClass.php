<?hh

class IllFormedClass {

  public static function foo(): void {
    // Normal dependency here, which we don't expect to be registered, since the overall file is ill-formed
    Root::root();
  }

  public static function broken(): void {
    // missing semicolon!
    return
  }

}
