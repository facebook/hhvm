<?hh // strict

namespace NS_MyVector_test;

require_once ('MyVector.php');

function main(): void {
  $iv1 = new \NS_MyVector\MyVector(7, 2);
  echo "\$iv1: >>$iv1<<\n";

  $iv1->setElement(1, 55);
  $iv1->setElement(3, $iv1->getElement(3) - 17);
  $iv1->setElement(5, $iv1->getElement(5) * 3);
  echo "\$iv1: >>$iv1<<\n";

  $sv1 = new \NS_MyVector\MyVector(5, 'X');
  echo "\$sv1: >>$sv1<<\n";

  $sv1->setElement(1, 'AB');
  $sv1->setElement(3, $sv1->getElement(4) . 'ZZ');
  echo "\$sv1: >>$sv1<<\n";

  $viv = new \NS_MyVector\MyVector(3, new \NS_MyVector\MyVector(2, -1));
  echo "\$viv: >>$viv<<\n";
  $viv->setElement(0, new \NS_MyVector\MyVector(2, 1));
  $viv->setElement(1, new \NS_MyVector\MyVector(4, 2));
  $viv->setElement(2, new \NS_MyVector\MyVector(3, 5));
  echo "\$viv: >>$viv<<\n";

  $res = f($iv1, $sv1, $viv);
  echo "\$res: >>$res<<\n";
  echo "\$viv: >>$viv<<\n";
}

function f(\NS_MyVector\MyVector<int> $p1, \NS_MyVector\MyVector<string> $p2,
  \NS_MyVector\MyVector<\NS_MyVector\MyVector<int>> $p3)
  : \NS_MyVector\MyVector<\NS_MyVector\MyVector<int>> {
  $p3->setElement(1, $p1);
  return $p3;
}

class Cx {
  private ?\NS_MyVector\MyVector<int> $pr1;
  private \NS_MyVector\MyVector<string> $pr2;
  private \NS_MyVector\MyVector<\NS_MyVector\MyVector<int>> $pr3;
  private \NS_MyVector\MyVector<?int> $pr4;
  private ?\NS_MyVector\MyVector<?int> $pr5;

  public function __construct() {
    $this->pr2 = new \NS_MyVector\MyVector(3, '??');
    $this->pr3 = new \NS_MyVector\MyVector(5, new \NS_MyVector\MyVector(3, 99));
    $this->pr4 = new \NS_MyVector\MyVector(2, null);
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();
