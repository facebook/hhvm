<?hh // experimental

for ($i = 1; $i < 10; $i = x) {
  let x = $i * 2;
  var_dump(x);
}
