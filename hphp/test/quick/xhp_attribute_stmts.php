<?hh

abstract class :base {
  private static $xhpAttributeDeclarationBase = darray[];

  protected static function __xhpAttributeDeclaration() {
    return self::$xhpAttributeDeclarationBase;
  }

  public static function xhpAttributeDeclaration() {
    return static::__xhpAttributeDeclaration();
  }

  public static function updateBase($d) {
    self::$xhpAttributeDeclarationBase = $d;
  }
}

class :node1 extends :base {
  attribute
    mixed attr1;
}

class :node2 extends :node1 {
  attribute
    int attr2,
    var attr3,
    mixed attr4,
    string attr5 = "hello";
}

class :node3 extends :base {
  attribute
    string attr2 = "world";
}

function repr($x) {
  if ($x === null) {
    return 'null';
  } else if ($x === true) {
    return 'true';
  } else if ($x === false) {
    return 'false';
  } else {
    return print_r($x, TRUE);
  }
}

function dump($arr) {
  foreach ($arr as $k => $v) {
    echo $k . "[" . join(",", array_map($x ==> repr($x), $v)) . "] ";
  }
  echo "\n";
}

dump(:node1::xhpAttributeDeclaration());
dump(:node2::xhpAttributeDeclaration());

// Verify that the value is cached
// Only node3 should have attr0
:base::updateBase(darray["attr0" => array(6, null, null, 0)]);
echo repr(array_key_exists("attr0", :base::xhpAttributeDeclaration())) . "\n";
echo repr(array_key_exists("attr0", :node1::xhpAttributeDeclaration())) . "\n";
echo repr(array_key_exists("attr0", :node2::xhpAttributeDeclaration())) . "\n";
echo repr(array_key_exists("attr0", :node3::xhpAttributeDeclaration())) . "\n";
