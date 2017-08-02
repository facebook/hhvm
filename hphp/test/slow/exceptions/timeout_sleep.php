<?hh

set_time_limit(1);

function foo() {
  echo "begin sleep\n";
  sleep(2);
  echo "end sleep\n";
}

foo();
