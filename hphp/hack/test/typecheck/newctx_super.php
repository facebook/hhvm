////file1.php
<?hh

<<file: __EnableUnstableFeatures('context_alias_declaration_short')>>
newctx MyContext as [];
newctx MyContextSuper as [] super [defaults];

////file2.php
<?hh

class Foo {
  public function f()[defaults]: void {} // explicit defaults for clarity
}

class Bar extends Foo {
  public function f()[MyContextSuper]: void {}
}

class Bad extends Foo {
  public function f()[MyContext]: void {}
}
