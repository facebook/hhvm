<?hh // experimental


<<__EntryPoint>>
function main_for_loop() {
for ($i = 0; $i < 5; $i++) {
  let twice = $i * 2;
  var_dump(twice);
}
}
