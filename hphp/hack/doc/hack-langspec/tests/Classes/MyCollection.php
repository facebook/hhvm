<?hh // strict

namespace NS_MyCollections;

interface MyCollection {
  public function put(int $item): void;
  public function get(): int;
}
