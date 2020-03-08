<?hh // strict

namespace NS_MyCollectionClasses;

interface MyCollection<T> {
  const int MAX_NUMBER_ITEMS = 1000;

  public function put(T $item): void;
  public function get(): T;
}
