<?hh // strict

namespace NS_MyCollectionClasses;

require_once 'MyCollection.php';

class MyQueue<T> implements MyCollection<T> {
  private array<T> $list = array();

  public function put(T $item): void {
    // ...
  }

  public function get(): T {
    // ...
    return $this->list[0];
  }
	
  // ...
}
