//// modules.php
<?hh

new module imod {}

//// iface.php
<?hh

module imod;

// Internal interface with bypass visibility
<<__TestsBypassVisibility>>
internal interface SecretInterface {
  public function exposed(): void;
}

//// impl.php
<?hh

module imod;

<<__TestsBypassVisibility>>
internal class SecretImpl implements SecretInterface {
  public function exposed(): void {}
}

//// test.php
<?hh

class WWWTest {}

class InterfaceTest extends WWWTest {
  public function test(): void {
    // Can we reference the internal interface type?
    $obj = new SecretImpl();
    $obj->exposed();
  }
}
