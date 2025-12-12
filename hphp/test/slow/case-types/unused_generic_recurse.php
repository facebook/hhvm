<?hh

<<file: __EnableUnstableFeatures('case_types')>>


case type C1<T1> = int;
case type C2<T2> = C1;
case type C3<T3> = C2;

case type D1<T1> = int;
case type D2<T2> = D1<T2>;
case type D3<T3> = D2<T3>;

case type E1<T1> = int;
     type E2<T2> = E1;
case type E3<T3> = E2;

case type F1<T1> = int;
     type F2<T2> = F1<T2>;
case type F3<T3> = F2<T3>;

type X = C3<bool>;
type Y = D3<string>;
type Z = E3<num>;
type Q = F3<null>;

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure_for_alias(nameof C3));
  var_dump(type_structure_for_alias(nameof D3));
  var_dump(type_structure_for_alias(nameof E3));
  var_dump(type_structure_for_alias(nameof F3));

  // The ordering of the alias, case_type, typevars, and typevar_types fields
  // are currently inconsistent between HackC, HHBBC, and HHVM here:
  //var_dump(type_structure_for_alias(nameof X));
  //var_dump(type_structure_for_alias(nameof Y));
  //var_dump(type_structure_for_alias(nameof Z));
  //var_dump(type_structure_for_alias(nameof Q));
}

