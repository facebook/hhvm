<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type HasGenericAndUsesIt<T> = T;

case type HasGenericAndDoesNotUseIt<T> = int;

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure_for_alias(nameof HasGenericAndUsesIt));
  var_dump(type_structure_for_alias(nameof HasGenericAndDoesNotUseIt));
}
