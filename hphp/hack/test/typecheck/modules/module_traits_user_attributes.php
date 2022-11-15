//// modules.php
<?hh
<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

class UserAttribute implements HH\ClassAttribute {
    public function __construct(mixed $x) {}
}
//// A.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module A;

internal class InternalToA {}

<<UserAttribute(InternalToA::class)>> // Ok
public trait PublicTraitA {}

//// B.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module B;

<<UserAttribute(InternalToA::class)>> // Error
public trait PublicTrait {}
