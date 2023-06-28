<?hh

// FILE: child.php

class Child extends Parent_ implements I {
  public function foo(): string {
    return __CLASS__;
  }

  public static function bar(): string {
    return "static ".__CLASS__;
  }
}

class GrandParent {
  public static function gp_do_bar(): string {
    return static::bar();
  }

  public function gp_call_foo(): string {
    return $this->foo();
  }

  public static function static_gp_call_foo($cls): string {
    return $cls->foo();
  }
}

interface I {
  public function foo(): string;
  public static function bar(): string;
}

// FILE: parent.php

class Parent_ extends GrandParent {
  use T;

  public static function p_do_bar(): string {
    return static::bar();
  }

  public function p_call_foo(): string {
    return $this->foo();
  }

  public static function static_p_call_foo($cls): string {
    return $cls->foo();
  }
}

// FILE: trait.php

trait T {
}

// FILE: parent.php VERSION: 1

class Parent_ extends GrandParent {
  use T;

  private function foo(): string {
    return __CLASS__;
  }

  private static function bar(): string {
    return "static ".__CLASS__;
  }

  public static function p_do_bar(): string {
    return static::bar();
  }

  public function p_call_foo(): string {
    return $this->foo();
  }

  public static function static_p_call_foo($cls): string {
    return $cls->foo();
  }
}

// FILE: parent.php VERSION: 2

class Parent_ extends GrandParent {
  use T;

  public static function p_do_bar(): string {
    return static::bar();
  }

  public function p_call_foo(): string {
    return $this->foo();
  }

  public static function static_p_call_foo($cls): string {
    return $cls->foo();
  }
}

// FILE: trait.php VERSION: 2

trait T {
  private function foo(): string {
    return __CLASS__." via T";
  }

  private static function bar(): string {
    return "static ".__CLASS__." via T";
  }
}

// FILE: main.php

<<__EntryPoint>>
function main() :mixed{
  $g = new GrandParent();
  $p = new Parent_();
  $c = new Child();

  $gc = GrandParent::class;
  $pc = Parent_::class;
  $cc = Child::class;

  var_dump($c->p_call_foo());
  var_dump($c->gp_call_foo());
  var_dump($cc::p_do_bar());
  var_dump($cc::gp_do_bar());

  var_dump($c->foo());
  var_dump($cc::bar());

  var_dump($gc::static_gp_call_foo($c));
  var_dump($pc::static_gp_call_foo($c));
  var_dump($pc::static_p_call_foo($c));
}


