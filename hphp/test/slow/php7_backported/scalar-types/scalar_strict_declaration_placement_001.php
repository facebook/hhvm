<?php

function takes_int(int $x) {

    if (Php7BackportedScalarTypesScalarStrictDeclarationPlacement001::$errored) {
        echo "Failure!", PHP_EOL;
        Php7BackportedScalarTypesScalarStrictDeclarationPlacement001::$errored = FALSE;
    } else {
        echo "Success!", PHP_EOL;
    }
}

?>
<?php

declare(strict_types=1);
var_dump(takes_int(32));

?>

abstract final class Php7BackportedScalarTypesScalarStrictDeclarationPlacement001 {
  public static $errored;
}
