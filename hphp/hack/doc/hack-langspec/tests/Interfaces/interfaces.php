<?hh // strict

namespace NS_interfaces;

require_once 'MyCollection.php';
require_once 'MyList.php';
require_once 'MyQueue.php';

function processCollection<T>(\NS_MyCollectionClasses\MyCollection<T> $p1): void {
  var_dump($p1);
}

interface iX {
  const int C1 = 123;
//  const string C2 = "green";

  public function f0(): void;
//  public function f1(int $p1): void;
//  private function f1(int $p1): void;	// private not permitted
//  protected function f1(int $p1): void;	// protected not permitted
  public static function f3(): void;
}

interface iY {
//  const int C1 = 123;		// can't inherit duplicate constants even if defined identically
  const string C2 = "green";

//  public function f0(int $p1): void;	// Declaration of iX::f0() must be compatible with iY::f0($p1)
  public function f1(int $p1): void;
  public function f2(int $p1, int $p2): void;
}

interface iZ extends iX, iY {
//  const int C1 = 123;		// can't override inherited constants
//  const string C2 = "green";	// can't override inherited constants

  public function f2(int $p1, int $p2): void;
}

abstract class C implements iZ {} // being abstract, it need not implement any of the methods

class D implements iZ {
//  public function f0(int $p1): void {} // Declaration of D::f0() must be compatible with iX::f0()
  public function f0(): void {}
  public function f1(int $p1): void {}
  public function f2(int $p1, int $p2): void {}
  public static function f3(): void {}
}

function main(): void {
  var_dump(D::C1);
  var_dump(D::C2);

  echo "------------------------------------\n";

  $list = new \NS_MyCollectionClasses\MyList();
  processCollection($list);

  $queue = new \NS_MyCollectionClasses\MyQueue();
  processCollection($queue);

  processCollection(new \NS_MyCollectionClasses\MyQueue());

  var_dump(\NS_MyCollectionClasses\MyCollection::MAX_NUMBER_ITEMS);
  var_dump(\NS_MyCollectionClasses\MyList::MAX_NUMBER_ITEMS);
  var_dump(\NS_MyCollectionClasses\MyQueue::MAX_NUMBER_ITEMS);
}

/* HH_FIXME[1002] call to main in strict*/
main();
