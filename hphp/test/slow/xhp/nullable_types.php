<?hh

class base {
  protected static function __xhpAttributeDeclaration()[]: darray {
    return dict[];
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


<<__EntryPoint>>
function main_nullable_types() :mixed{
:foo::dumpAttributes();
}
