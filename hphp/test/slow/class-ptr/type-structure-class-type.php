<?hh

<<file:__EnableUnstableFeatures('class_type')>>

type T = class<C>;
type U = classname<C>;
type V = class_or_classname<C>;
class C {
  const type T = class<C>;
  const type U = classname<C>;
  const type V = class_or_classname<C>;
}

<<__EntryPoint>>
function main(): void {
  $t = type_structure('T');
  $u = type_structure('U');
  $v = type_structure('V');
  $ct = type_structure('C', 'T');
  $cu = type_structure('C', 'U');
  $cv = type_structure('C', 'V');

  invariant($t['kind'] === TypeStructureKind::OF_CLASS_PTR, 'kind mismatch');
  invariant($u['kind'] === TypeStructureKind::OF_STRING, 'kind mismatch');
  invariant($v['kind'] === TypeStructureKind::OF_CLASS_OR_CLASSNAME, 'kind mismatch');
  invariant($ct['kind'] === TypeStructureKind::OF_CLASS_PTR, 'kind mismatch');
  invariant($cu['kind'] === TypeStructureKind::OF_STRING, 'kind mismatch');
  invariant($cv['kind'] === TypeStructureKind::OF_CLASS_OR_CLASSNAME, 'kind mismatch');

  var_dump($t);
  var_dump($u);
  var_dump($v);
  var_dump($ct);
  var_dump($cu);
  var_dump($cv);
}
