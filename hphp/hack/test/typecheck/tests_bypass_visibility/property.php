<?hh

class WWWTest {}

class WithProps {
  <<__TestsBypassVisibility>>
  private int $priv_prop = 0;

  <<__TestsBypassVisibility>>
  protected string $prot_prop = "hello";

  <<__TestsBypassVisibility>>
  private static int $spriv_prop = 0;
}

class PropTest extends WWWTest {
  public function test(WithProps $obj): void {
    $_ = $obj->priv_prop; // read private property
    $obj->priv_prop = 42; // write private property
    $_ = $obj->prot_prop; // read protected property
    $_ = WithProps::$spriv_prop; // read static private property
  }
}

class PropNonTest {
  public function test(WithProps $obj): void {
    $_ = $obj->priv_prop; // error
    $_ = $obj->prot_prop; // error
  }
}
