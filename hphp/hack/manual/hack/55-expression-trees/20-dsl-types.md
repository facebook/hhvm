For every literal supported in a DSL, the visitor must provide a stub method that shows what type it should have.

```Hack file:types.hack
class MyDsl {
  // Types for literals
  public static function intType(): MyDslInt {
    throw new Exception();
  }
  public static function floatType(): MyDslFloat {
    throw new Exception();
  }
  public static function boolType(): MyDslBool {
    throw new Exception();
  }
  public static function stringType(): MyDslString {
    throw new Exception();
  }

  // Visitors can use the normal Hack types if they wish.
  // This is particularly common for null and void.
  public static function nullType(): null {
    throw new Exception();
  }
  public static function voidType(): void {
    throw new Exception();
  }

  // symbolType is used in function calls.
  // In this example, MyDsl is using normal Hack
  // functions but DSLs may choose more specific APIs.
  public static function symbolType<T>(
    T $_,
  ): T {
    throw new Exception();
  }
}
```

All types used in the visitor need to define what operators they support. Here's an expanded version of `MyDsl` with types that are very similar to plain Hack.

```Hack file:types.hack
interface MyDslNonnull {
  public function __tripleEquals(?MyDslNonnull $_): MyDslBool;
  public function __notTripleEquals(?MyDslNonnull $_): MyDslBool;
}
interface MyDslNum extends MyDslNonnull {
  // Arithmetic
  public function __plus(MyDslNum $_): MyDslNum;
  public function __minus(MyDslNum $_): MyDslNum;
  public function __star(MyDslNum $_): MyDslNum;
  public function __slash(MyDslNum $_): MyDslNum;
  public function __negate(): MyDslNum;

  // Comparisons
  public function __lessThan(MyDslNum $_): MyDslBool;
  public function __lessThanEqual(MyDslNum $_): MyDslBool;
  public function __greaterThan(MyDslNum $_): MyDslBool;
  public function __greaterThanEqual(MyDslNum $_): MyDslBool;
}
interface MyDslInt extends MyDslNum {
  // Only support modulus on integers in our demo.
  public function __percent(MyDslInt $_): MyDslInt;

  // Bitwise operators
  public function __amp(MyDslInt $_): MyDslInt;
  public function __bar(MyDslInt $_): MyDslInt;
  public function __caret(MyDslInt $_): MyDslInt;
  public function __lessThanLessThan(MyDslInt $_): MyDslInt;
  public function __greaterThanGreaterThan(MyDslInt $_): MyDslInt;
  public function __tilde(): MyDslInt;
}
interface MyDslFloat extends MyDslNum {
  <<__Override>>
  public function __plus(MyDslNum $_): MyDslFloat;
}
interface MyDslString extends MyDslNonnull {
  public function __dot(MyDslString $_): MyDslString;
}
interface MyDslBool extends MyDslNonnull {
  // __bool signifies that we can use MyDslBool in positions that require
  // a truthy value, such as if statements.
  public function __bool(): bool;
  // Infix operators that return another MyDslBool.
  public function __ampamp(MyDslBool $_): MyDslBool;
  public function __barbar(MyDslBool $_): MyDslBool;

  public function __exclamationMark(): MyDslBool;
}
```
