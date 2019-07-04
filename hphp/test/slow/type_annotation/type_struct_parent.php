<?hh

interface IThis {
  const type TThis = this;
}

require 'type_struct_parent.inc';

require 'type_struct_child.inc';
<<__EntryPoint>> function main(): void {
var_dump(type_structure('IThis', 'TThis'));

var_dump(type_structure('P', 'TThis'));

var_dump(type_structure('C', 'TThis'));
}
