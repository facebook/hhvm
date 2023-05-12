//// defaults.php
<?hh
namespace HH\Contexts {
  type defaults = (I);
  interface I {}
}

//// Foo.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Foo as [defaults];

//// Arraykey.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Arraykey as [defaults];

//// Bool.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Bool as [defaults];

//// Callable.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Callable as [defaults];

//// Classname.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Classname as [defaults];

//// Darray.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Darray as [defaults];

//// Dynamic.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Dynamic as [defaults];

//// False.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx False as [defaults];

//// Float.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Float as [defaults];

//// Int.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Int as [defaults];

//// Mixed.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Mixed as [defaults];

//// Nonnull.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Nonnull as [defaults];

//// Noreturn.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Noreturn as [defaults];

//// Nothing.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Nothing as [defaults];

//// Null.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Null as [defaults];

//// Num.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Num as [defaults];

//// Parent.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Parent as [defaults];

//// Resource.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Resource as [defaults];

//// Self.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Self as [defaults];

//// Static.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Static as [defaults];

//// String.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx String as [defaults];

//// This.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx This as [defaults];

//// True.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx True as [defaults];

//// Varray.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Varray as [defaults];

//// Void.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx Void as [defaults];

//// _.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
newctx _ as [defaults];
