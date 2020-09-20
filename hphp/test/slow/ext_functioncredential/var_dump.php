<?hh

class TestClass {
  public static function static_method(): FunctionCredential {
    return __FUNCTION_CREDENTIAL__;
  }
}

<<__EntryPoint>>
function main(): void {
  var_dump(__FUNCTION_CREDENTIAL__);
  var_dump(TestClass::static_method());
}
