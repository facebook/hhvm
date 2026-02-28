<?hh

function foo() :mixed{
  echo "begin sleep\n";
  sleep(2);
  echo "end sleep\n";
}


<<__EntryPoint>>
function main_timeout_sleep() :mixed{
set_time_limit(1);

foo();
}
