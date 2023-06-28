<?hh

interface I1 {}


<<__EntryPoint>>
function main_require_constraint_iface_implements_error() :mixed{
if (time() > 0) {
  include 'require_constraint_iface_implements_error.inc';
}

echo 'Fail';
}
