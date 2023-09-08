//// mod.php
<?hh

new module b {}
//// def.php
<?hh

module b;

internal class SecretKey{
  public function get(): int {
    return 5;
  }
}
internal class SecretKey2 extends SecretKey  {}

// Public trait, here.
public trait SecretHolder<T, Tv> {
  // Ooops, we shouldn't have made this public, but we did...
  public T $secret;
  public Tv $secret2;
}

public class User {
  use SecretHolder<SecretKey, SecretKey2>;

  public function __construct() {
    $this->secret = new SecretKey();
    $this->secret2 = new SecretKey2();
  }

  public function returnInternalBad(): SecretKey { // error, do not allow in public class
    return new SecretKey();
  }
}
internal class X extends User {
  public function returnInternal(): SecretKey {
    return new SecretKey();
  }
}

class Y extends X {}



//// file.php
<?hh

function test(User $x): void {
    $x->secret = $x->secret;
    $a = $x->secret; // ok, but opaque
    $a->get();

    $y = new Y();
    $b = $y->returnInternal(); // ok, but opaque
    $b->get();
}
