<?hh

interface I {
  public static function sm(): arraykey;
}
interface J {
  public static function sm(): num;
}

function f<reify T as I as J>(): void {
  T::sm();
}
