<?

#error_reporting(E_NONE);

class Foo {
  private static $foo = "I'm a foo";
  private $myFoo = "I'm a foo instance";

  private function instanceDo() { return 'Foo::instanceDo'; }
  private static function staticDo() { return 'Foo::staticDo'; }

  function getClosure() {
    return function () {
      if (!isset($this)) {
        echo "No \$this\n";
        return;
      }

      if (isset($this->myFoo)) {
        echo '$this->myFoo = ';
        var_dump($this->myFoo);
      }

      if (isset(static::$foo)) {
        echo 'static::$foo = ';
        var_dump(static::$foo);
      }

      if (isset(self::$foo)) {
        echo 'self::$foo = ';
        var_dump(self::$foo);
      }

      if (method_exists($this, 'staticDo')) {
        echo 'static::staticDo() = ';
        var_dump(static::staticDo());
      }

      if (method_exists($this, 'instanceDo')) {
        echo '$this->instanceDo() = ';
        var_dump($this->instanceDo());
      }
    };
  }

  static function getStaticClosure() {
    return static function () {
      if (isset(static::$foo)) {
        echo 'static::$foo = ';
        var_dump(static::$foo);
      }

      if (isset(self::$foo)) {
        echo 'self::$foo = ';
        var_dump(self::$foo);
      }

      if (method_exists(get_called_class(), 'staticDo')) {
        echo 'static::staticDo() = ';
        var_dump(static::staticDo());
      }
    };
  }
}

class Bar {
  private static $foo = "I'm a bar";
  private $myFoo = "I'm a bar instance";

  private function instanceDo() { return 'Bar::instanceDo'; }
  private function staticDo() { return 'Bar::staticDo'; }
}

class Baz {}

$foo = new Foo;
$bar = new Bar;
$baz = new Baz;

$staticCl = Foo::getStaticClosure();
$Cl = $foo->getClosure();

echo "================================================================\n";
echo "\$staticCl()\n";
@$staticCl();
echo "\$Cl()\n";
@$Cl();


echo "================================================================\n";
echo "\$s1 = \$staticCl->bindTo(NULL, 'Bar')\n";
$s1 = $staticCl->bindTo(NULL, 'Bar');
@$s1();
echo "\$s2 = \$Cl->bindTo(\$bar, 'Bar')\n";
$s2 = $Cl->bindTo($bar, 'Bar');
@$s2();


echo "================================================================\n";
echo "\$s1 = \$staticCl->bindTo(NULL, 'Baz')\n";
$s1 = $staticCl->bindTo(NULL, 'Baz');
$s1();
echo "\$s2 = \$Cl->bindTo(\$baz, 'Baz')\n";
$s2 = $Cl->bindTo($baz, 'Baz');
$s2();


echo "================================================================\n";
echo "\$s2 = \$Cl->bindTo(\$baz, 'Bar')\n";
$s2 = $Cl->bindTo($baz, 'Bar');
$s2();


echo "================================================================\n";
echo "\$s2 = \$Cl->bindTo(\$foo, 'Baz')\n";
$s2 = $Cl->bindTo($foo, 'Baz');
$s2();
