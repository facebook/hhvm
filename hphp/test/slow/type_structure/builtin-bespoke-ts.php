<?hh

class C {
  const type TExact = this;
}

type TBool = bool;
type TFloat = float;
type TString = ?@~string;
newtype TNum = num;

type TShape = shape('a' => bool, 'b' => int);
type TTuple = (mixed, int);
type TTypevar<T> = T;

type TGenericDict<T> = dict<int, T>;
type TBoolDict = TGenericDict<bool>;

type TClass = C;

type TFun1 = (function (darray<string, mixed>, int...): bool);
type TFun2 = (function (): void);

<<__EntryPoint>>
function main(): void {
  $ts = type_structure('TBool');
  var_dump(HH\TypeStructure\get_kind($ts));

  $ts = type_structure('TString');
  var_dump(HH\TypeStructure\get_nullable($ts));
  var_dump(HH\TypeStructure\get_soft($ts));
  var_dump(HH\TypeStructure\get_opaque($ts));
  var_dump(HH\TypeStructure\get_optional_shape_field($ts));

  $ts = type_structure('TNum');
  var_dump(HH\TypeStructure\get_nullable($ts));
  var_dump(HH\TypeStructure\get_opaque($ts));
  var_dump(HH\TypeStructure\get_alias($ts));
  var_dump(HH\TypeStructure\get_typevars($ts));

  $ts = type_structure('TShape');
  var_dump(HH\TypeStructure\get_fields($ts));
  var_dump(HH\TypeStructure\get_elem_types($ts) ?? "not in shape");
  var_dump(HH\TypeStructure\get_allows_unknown_fields($ts));

  $ts = type_structure('TTuple');
  var_dump(HH\TypeStructure\get_elem_types($ts));

  $ts = type_structure('TTypevar', null);
  var_dump(HH\TypeStructure\get_name($ts));

  $ts = type_structure('TBoolDict', null);
  var_dump(HH\TypeStructure\get_alias($ts));
  var_dump(HH\TypeStructure\get_typevar_types($ts));
  var_dump(HH\TypeStructure\get_generic_types($ts));
  var_dump(HH\TypeStructure\get_fields($ts) ?? "not in generic");

  $ts = type_structure('TClass', null);
  var_dump(HH\TypeStructure\get_classname($ts));
  var_dump(HH\TypeStructure\get_generic_types($ts) ?? "no generic types");

  $ts = type_structure('C', 'TExact');
  var_dump(HH\TypeStructure\get_exact($ts));

  $ts = type_structure('TFun1');
  var_dump(HH\TypeStructure\get_return_type($ts));
  var_dump(HH\TypeStructure\get_param_types($ts));
  var_dump(HH\TypeStructure\get_variadic_type($ts));

  $ts = type_structure('TFun2');
  var_dump(HH\TypeStructure\get_return_type($ts));
  var_dump(HH\TypeStructure\get_param_types($ts));
}
