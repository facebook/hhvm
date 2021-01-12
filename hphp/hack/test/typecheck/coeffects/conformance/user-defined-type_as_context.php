<?hh

class Clazz {
  public function unqualified_class_bad()[Clazz]: void {}
}
interface Iface {
  public function unqualified_interface_bad()[Iface]: void;
}

function unqualified_class_bad()[Clazz]: void {}
function unqualified_interface_bad()[Iface]: void {}
