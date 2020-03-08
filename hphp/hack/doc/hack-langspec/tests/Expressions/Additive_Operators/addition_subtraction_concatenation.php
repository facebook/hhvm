<?hh // strict

namespace NS_addition_subtraction_concatenation;

function doit(num $p1_num): void {
  $oper = array(100, -3.4, $p1_num);
///*
  foreach ($oper as $e1) {
    foreach ($oper as $e2) {
      echo ">$e1< + >$e2<, result: "; var_dump($e1 + $e2);
    }
    echo "-------------------------------------\n";
  }
//*/
///*
  foreach ($oper as $e1) {
    foreach ($oper as $e2) {
      echo ">$e1< - >$e2<, result: "; var_dump($e1 - $e2);
    }
    echo "-------------------------------------\n";
  }
//*/
///*
  $oper = array(100, -3.4, true, null, "123", "2e+5", "", "abc");
  foreach ($oper as $e1) {
    foreach ($oper as $e2) {
      echo ">$e1< . >$e2<, result: "; var_dump($e1 . $e2);
    }
    echo "-------------------------------------\n";
  }
//*/
}

function main(): void {
  doit(11);
}

/* HH_FIXME[1002] call to main in strict*/
main();
