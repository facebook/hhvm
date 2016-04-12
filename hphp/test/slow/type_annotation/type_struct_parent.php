<?hh

interface IThis {
  const type TThis = this;
}

var_dump(type_structure('IThis', 'TThis'));

require 'type_struct_parent.inc';

var_dump(type_structure('P', 'TThis'));

require 'type_struct_child.inc';

var_dump(type_structure('C', 'TThis'));
