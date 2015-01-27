<?hh

class T<T> {
  public static function f() {
    return new T();
  }
}
