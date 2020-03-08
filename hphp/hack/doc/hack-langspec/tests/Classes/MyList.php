<?hh // strict

namespace NS_MyList;

require_once 'MyCollection.php';

class MyList implements \NS_MyCollections\MyCollection {
  public function put(int $item): void {
    // ...
  }

  public function get(): int {
    // ...
    return 100;
  }
	
  // ...
}

