<?hh

type C1 = (int, C1);

<<__EntryPoint>>
function main() :mixed{
  type_structure_for_alias('C1');
}
