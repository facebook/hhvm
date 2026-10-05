//// module.php
<?hh
new module A {}

//// internal.php
<?hh
module A;

internal final class InternalClass {}

//// use.php
<?hh
<<file:__EnableUnstableFeatures('simpli_hack', 'shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

<<__SimpliHack(shape(...shape('class' => InternalClass::class)))>>
function f(): void {}
