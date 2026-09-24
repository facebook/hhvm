//// base-declaration.php
<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class NamedParameterTarget {
  public static function call(named int $old_name): void {}
}
//// base-caller.php
<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function call_named_parameter_target(): void {
  NamedParameterTarget::call(old_name=1);
}

//// changed-declaration.php
<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

final class NamedParameterTarget {
  public static function call(named int $new_name): void {}
}
//// changed-caller.php
<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function call_named_parameter_target(): void {
  NamedParameterTarget::call(old_name=1);
}
