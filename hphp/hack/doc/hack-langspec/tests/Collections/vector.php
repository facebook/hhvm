<?hh // strict

namespace NS_vector;

class C {
  public array<int> $pr1;
  public function __construct() {
    $this->pr1 = array(22, 33, 44);
  }
}

function displayVector<T>(Vector<T> $v): void {
  var_dump($v);
/*
  foreach ($v as $item) {
//    var_dump($item);
      echo (string)$item . "\n";
  }
*/
}

function main(): void {
  $v1 = new Vector(null);		// vector of zero items
  displayVector($v1);
  echo "\$v1 = " . ($v1->isEmpty() ? "is" : "is not") . " empty\n";

  $v1 = new Vector(array());	// vector of zero items
  displayVector($v1);

  $v1 = Vector {};		// vector of zero items
  echo "\$v1's element count = " . $v1->count() . "\n";
  displayVector($v1);

  $v1->add(12);			// add item 0
  $v1[] = 99;			// add item 1
//  $v1[2] = 2001;			// add item 2 ==> OutOfBoundsException
  $v1[] = 199;			// add item 2
  displayVector($v1);
  echo "\$v1's element count = " . $v1->count() . "\n";
  echo "\$v1 = " . ($v1->isEmpty() ? "is" : "is not") . " empty\n";

  if ($v1->containsKey(1)) {
    echo "\$v1[1] = " . $v1->at(1) . "\n";
  }
//  echo "\$v1[-1] = " . $v1->at(-1) . "\n";	// OutOfBoundsException
//  echo "\$v1[100] = " . $v1->at(100) . "\n";	// OutOfBoundsException

  $v2 = Vector::fromItems(array(2, 8));
  displayVector($v2);

  $v2 = $v1->filter(function (int $value): bool {
    return $value > 50;
  });
  displayVector($v2);

  $v1->clear();

  $v1->addAll(null);			// add no new items

  $c1 = new C();
  var_dump($c1);

  $v1->addAll($c1->pr1);		// add 3 new items
  displayVector($v1);

  $c1->pr1[-5] = 17;
  $c1->pr1[200] = 18;
  var_dump($c1->pr1);
  $v1 = new Vector($c1->pr1);
  displayVector($v1);
  $v1->addAll($c1->pr1);
  displayVector($v1);
  echo "\$v1 contains >$v1<\n";

  $v2 = Vector::fromItems($c1->pr1);
  displayVector($v2);

  $v2 = Vector {-100, -50, 0, 50, 100};
  $v2->removeKey(1);
  $v2->removeKey(3);
  displayVector($v2);

  $v2->reverse();
  displayVector($v2);

  $str = serialize($v2);
  echo "\$str contains >$str<\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
