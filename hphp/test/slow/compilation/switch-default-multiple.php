<?hh


<<__EntryPoint>>
function main_switch_default_multiple() {
switch (time()) {
  default:
    echo "First default executed\n";
    break;
  default:
    echo "Second default executed\n";
}
}
