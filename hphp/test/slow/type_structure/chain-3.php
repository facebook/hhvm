<?hh

type No<Tno> = int;
type Yes<Tyes> = Tyes;
type NoNo<Tnono> = No<float>;
type NoYes<Tnoyes> = Yes<bool>;
type YesNo<Tyesno> = No<Tyesno>;
type YesYes<Tyesyes> = Yes<Tyesyes>;

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure_for_alias(nameof No));
  var_dump(type_structure_for_alias(nameof Yes));
  var_dump(type_structure_for_alias(nameof NoNo));
  var_dump(type_structure_for_alias(nameof NoYes));
  var_dump(type_structure_for_alias(nameof YesNo));
  var_dump(type_structure_for_alias(nameof YesYes));
}
