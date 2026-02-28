<?hh

<<file: __EnableUnstableFeatures('case_types')>>
<<file: __EnableUnstableFeatures('case_type_where_clauses')>>

case type MyCaseType<T> =
  | T
  | int where T super int
  | string where T as nothing;

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure_for_alias(MyCaseType::class));
}
