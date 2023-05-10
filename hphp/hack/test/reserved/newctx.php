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

//// NFoo.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Foo as [defaults];

//// NArraykey.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Arraykey as [defaults];

//// NBool.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Bool as [defaults];

//// NCallable.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Callable as [defaults];

//// NClassname.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Classname as [defaults];

//// NDarray.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Darray as [defaults];

//// NDynamic.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Dynamic as [defaults];

//// NFalse.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx False as [defaults];

//// NFloat.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Float as [defaults];

//// NInt.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Int as [defaults];

//// NMixed.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Mixed as [defaults];

//// NNonnull.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Nonnull as [defaults];

//// NNoreturn.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Noreturn as [defaults];

//// NNothing.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Nothing as [defaults];

//// NNull.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Null as [defaults];

//// NNum.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Num as [defaults];

//// NParent.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Parent as [defaults];

//// NResource.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Resource as [defaults];

//// NSelf.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Self as [defaults];

//// NStatic.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Static as [defaults];

//// NString.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx String as [defaults];

//// NThis.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx This as [defaults];

//// NTrue.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx True as [defaults];

//// NVarray.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Varray as [defaults];

//// NVoid.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx Void as [defaults];

//// N_.php
<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration_short')>>
namespace N;
newctx _ as [defaults];
