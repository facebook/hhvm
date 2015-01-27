<?php

class Y {
  public static $x = 'in Y (static)';
  public $y = 'in Y (instance)';
}

class X {
  public static $x = 'in X (static)';
  public $y = 'in X (instance)';

  public static function getS() {
    return static function() {
      echo "self::\$x   = "; var_dump(self::$x);
      echo "static::\$x = "; var_dump(static::$x);
    };
  }

  public function get() {
    return function() {
      echo "\$this->y   = "; var_dump($this->y);
      echo "self::\$x   = "; var_dump(self::$x);
      echo "static::\$x = "; var_dump(static::$x);
    };
  }
}

echo "\$d = \$staticNS->bindto(NULL, 'Y');\n";
$staticNS = static function() {
  echo "self::\$x   = "; var_dump(self::$x);
  echo "static::\$x = "; var_dump(static::$x);
};
$d = $staticNS->bindto(NULL, 'Y');
$res = (new ReflectionFunction($d))->getClosureScopeClass();
if ($res) {
  var_dump($res->getName());
}

$d();

echo "=====================================================\n";
echo "\$d = \$staticS->bindto(NULL, 'Y');\n";
$staticS  = X::getS();
$d = $staticS->bindto(NULL, 'Y');
$res = (new ReflectionFunction($d))->getClosureScopeClass();
if ($res) {
  var_dump($res->getName());
}

$d();

echo "=====================================================\n";
echo "\$R = \$Q->bindto(new Y, 'static');\n";

$Q = (new X)->get();
$R = $Q->bindto(new Y, 'static');
$res = (new ReflectionFunction($R))->getClosureScopeClass();
if ($res) {
  var_dump($res->getName());
}
$R();

echo "=====================================================\n";
echo "\$R = \$Q->bindto(new Y, 'X');\n";

$Q = (new X)->get();
$R = $Q->bindto(new Y, 'X');
$res = (new ReflectionFunction($R))->getClosureScopeClass();
if ($res) {
  var_dump($res->getName());
}

$R();

echo "=====================================================\n";
echo "\$R = \$Q->bindto(new Y, 'Y');\n";

$Q = (new X)->get();
$R = $Q->bindto(new Y, 'Y');
$res = (new ReflectionFunction($R))->getClosureScopeClass();
if ($res) {
  var_dump($res->getName());
}
$R();
