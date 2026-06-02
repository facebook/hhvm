<?hh
module a;

<<__TestsBypassVisibility>>
internal class InternalTarget {
  public static function greet(): string { return "greet"; }
}
