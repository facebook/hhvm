<?hh

class base {
  protected static function __xhpAttributeDeclaration(): array {
    return [];
  }
}

class :foo extends base {
  attribute
    string mystring @required,
    ?string mynullablestring,
    MyClass myobject @required,
    ?MyClass mynullableobject,
    int myint @required,
    ?int mynullableint @required;


  public static function dumpAttributes(): void {
    /* [
     *    [attr type, class name or enum values, default, required],
     *    ...
     * ]
     */
    var_dump(self::__xhpAttributeDeclaration());
  }
}

:foo::dumpAttributes();
