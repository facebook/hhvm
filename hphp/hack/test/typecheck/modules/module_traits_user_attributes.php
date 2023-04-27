//// modules.php
<?hh


new module A {}
new module B {}

class UserAttribute implements HH\ClassAttribute {
    public function __construct(mixed $x) {}
}
//// A.php
<?hh


module A;

internal class InternalToA {}

<<UserAttribute(InternalToA::class)>> // Ok
public trait PublicTraitA {}

//// B.php
<?hh


module B;

<<UserAttribute(InternalToA::class)>> // Error
public trait PublicTrait {}
