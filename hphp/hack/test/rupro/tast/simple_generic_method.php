<?hh

interface IBox<T> {
  public function set(T $x): void;
  public function get(): T;
}

interface IBoxFactory {
  public function makeBox<T>(): IBox<T>;
}
