<?hh

abstract class :base {
  // This is necessary because the generated __xhpAttributeDeclaration() has a
  // call to parent::__xhpAttributeDeclaration().
  protected static function __xhpAttributeDeclaration()[] :mixed{
    return dict[];
  }

  public static function xhpAttributeDeclaration() :mixed{
    return static::__xhpAttributeDeclaration();
  }
}

class :node1 extends :base {
  attribute
    mixed checkme;
}

class :node2 extends :base {
  attribute
    Exception e,
    mixed checkme;
}

class :node3 extends :base {
  attribute
    int num,
    mixed checkme,
    var beans,
    string cheese;
}

class :node4 extends :base {
  attribute
    int num = 2,
    mixed checkme,
    var beans,
    string cheese = "hi";
}

function test($x) :mixed{
  echo "====================================================\n";
  var_dump($x);
  var_dump(is_array($x));
  var_dump(is_varray($x));
  var_dump(is_darray($x));
  var_dump(is_vec($x));
  var_dump(is_dict($x));
}


<<__EntryPoint>>
function main_xhp() :mixed{
test(:node1::xhpAttributeDeclaration());
test(:node2::xhpAttributeDeclaration());
test(:node3::xhpAttributeDeclaration());
test(:node4::xhpAttributeDeclaration());
}
