<?hh

class T<T> {
  public static function f(): T {
    return new T();
  }
}
