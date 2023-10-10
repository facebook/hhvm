<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class base {
  protected static function __xhpAttributeDeclaration()[]: darray {
    return darray[];
  }
}

class :foo extends base {
  attribute string a @required;
  attribute string b @lateinit;
  attribute string c;
  public static function attr_decl(): void {
    var_dump(self::__xhpAttributeDeclaration());
  }
}

<<__EntryPoint>>
function main_rquired_attr() :mixed{
  :foo::attr_decl();
}
