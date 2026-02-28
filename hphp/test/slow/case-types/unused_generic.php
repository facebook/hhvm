<?hh

<<file: __EnableUnstableFeatures('case_types')>>

case type CaseHasGenericAndUsesIt<T> = T;

type HasGenericAndUsesIt<T> = T;

case type CaseHasGenericAndDoesNotUseIt<T> = int;

type HasGenericAndDoesNotUseIt<T> = int;

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure_for_alias(nameof HasGenericAndUsesIt));
  var_dump(type_structure_for_alias(nameof CaseHasGenericAndUsesIt));
  var_dump(type_structure_for_alias(nameof HasGenericAndDoesNotUseIt));
  var_dump(type_structure_for_alias(nameof CaseHasGenericAndDoesNotUseIt));
}
