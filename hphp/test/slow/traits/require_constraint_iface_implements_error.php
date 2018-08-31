<?hh

interface I1 {}


<<__EntryPoint>>
function main_require_constraint_iface_implements_error() {
if (time() > 0) {
  interface I2 {
    require implements I1;
  }
}

echo 'Fail';
}
