<?hh

function main() :mixed{
  $io = 1;
  $do = 1.5;

  $it = 2;
  $dt = 2.3;

  $ix = 10;
  $dx = 10.7;

  $io **= $ix;
  var_dump($io);
  $dx **= $dx;
  var_dump($dx);

  $it **= $dt;
  var_dump($it);
  $dt **= $it;
  var_dump($dt);
}


<<__EntryPoint>>
function main_power_assign() :mixed{
main();
}
