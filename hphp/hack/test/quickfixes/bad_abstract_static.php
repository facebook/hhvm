<?hh

abstract class MyClass {
    abstract public static function childImplements(): void;

    public static function foo(): void {
    self::childImplements();
    }
}