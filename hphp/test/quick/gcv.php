<?php
class C {
  public $pub = 'C::$pub';
  protected $prot = 'C::$prot';
  private $priv = 'C::$priv';

  public static function show() {
    echo "in C: accessing C\n";
    var_dump(get_class_vars('C'));

    echo "in C: accessing D\n";
    var_dump(get_class_vars('D'));

    echo "in C: accessing E\n";
    var_dump(get_class_vars('E'));
  }
}

class D extends C {
  public $pub_from_d = 'D::$pub_from_d';
  protected $prot_from_d = 'D::$prot_from_d';
  private $priv_from_d = 'D::$priv_from_d';

  public static function show() {
    echo "in D: accessing C\n";
    var_dump(get_class_vars('C'));

    echo "in D: accessing D\n";
    var_dump(get_class_vars('D'));

    echo "in D: accessing E\n";
    var_dump(get_class_vars('E'));
  }
}

class E extends D {
  private $priv = 'E::$priv'; // same name as C::$priv

  public static function show() {
    echo "in E: accessing C\n";
    var_dump(get_class_vars('C'));

    echo "in E: accessing D\n";
    var_dump(get_class_vars('D'));

    echo "in E: accessing E\n";
    var_dump(get_class_vars('E'));
  }
}


class SC {
  public static $pub = 'SC::$pub';
  protected static $prot = 'SC::$prot';
  private static $priv = 'SC::$priv';

  public static function show() {
    echo "in SC: accessing SC\n";
    var_dump(get_class_vars('SC'));

    echo "in SC: accessing SD\n";
    var_dump(get_class_vars('SD'));

    echo "in SC: accessing SE\n";
    var_dump(get_class_vars('SE'));
  }
}

class SD extends SC {
  public static $pub_from_d = 'SD::$pub_from_d';
  protected static $prot_from_d = 'SD::$prot_from_d';
  private static $priv_from_d = 'SD::$priv_from_d';

  public static function show() {
    echo "in SD: accessing SC\n";
    var_dump(get_class_vars('SC'));

    echo "in SD: accessing SD\n";
    var_dump(get_class_vars('SD'));

    echo "in SD: accessing SE\n";
    var_dump(get_class_vars('SE'));
  }
}

class SE extends SD {
  private static $priv = 'SE::$priv'; // same name as SC::$priv

  public static function show() {
    echo "in SE: accessing SC\n";
    var_dump(get_class_vars('SC'));

    echo "in SE: accessing SD\n";
    var_dump(get_class_vars('SD'));

    echo "in SE: accessing SE\n";
    var_dump(get_class_vars('SE'));
  }
}


echo ">>> testing instance variables from global scope\n";
echo "accessing C\n";
var_dump(get_class_vars('C'));

echo "accessing D\n";
var_dump(get_class_vars('D'));

echo "accessing E\n";
var_dump(get_class_vars('E'));

echo ">>> testing instance variables from class scope\n";
C::show();
D::show();
E::show();

echo ">>> testing static variables from global scope\n";
echo "accessing SC\n";
var_dump(get_class_vars('SC'));

echo "accessing SD\n";
var_dump(get_class_vars('SD'));

echo "accessing SE\n";
var_dump(get_class_vars('SE'));

echo ">>> testing static variables from class scope\n";
SC::show();
SD::show();
SE::show();

