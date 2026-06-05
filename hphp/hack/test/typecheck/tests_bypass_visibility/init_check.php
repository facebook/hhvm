<?hh

class WWWTest {}

class NormalPrivInit {
  private int $x;

  public function __construct() {
    // Normal private: init checker walks into the method body
    // and sees that $this->x is set, so no error
    $this->initX();
  }

  private function initX(): void {
    $this->x = 42;
  }
}

class BypassPrivInit {
  private int $x;

  public function __construct() {
    // With __TestsBypassVisibility: treated as non-private for init checking.
    // Init checker does NOT walk into the method body, so it conservatively
    // requires all properties to be initialized before the call.
    // This means $this->x must be set BEFORE calling initX().
    $this->initX();
  }

  <<__TestsBypassVisibility>>
  private function initX(): void {
    $this->x = 42;
  }
}

class BypassPrivInitFixed {
  private int $x;

  public function __construct() {
    // Fixed: set property before calling the bypass-visibility method
    $this->x = 0;
    $this->initX();
  }

  <<__TestsBypassVisibility>>
  private function initX(): void {
    $this->x = 42;
  }
}
