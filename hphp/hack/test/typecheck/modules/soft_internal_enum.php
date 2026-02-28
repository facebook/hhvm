//// def.php
<?hh
new module foo {}
//// use.php
<?hh
module foo;
<<__SoftInternal>>
internal enum FooEnum : int {}
<<__SoftInternal>>
internal enum class FooEnumCls : int {}
