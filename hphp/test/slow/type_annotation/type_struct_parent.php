<?hh

interface IThis {
  const type TThis = this;
}

<<__EntryPoint>> function main(): void {
  require 'type_struct_parent.inc';
  require 'type_struct_child.inc';

  var_dump(type_structure('IThis', 'TThis'));

  var_dump(type_structure('P', 'TThis'));

  var_dump(type_structure('C', 'TThis'));
}
