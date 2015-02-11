<?php

function sleep2() {
  sleep(2);
}

ini_set('hhvm.max_cpu_time', 1);
sleep2();
echo "done 1\n";

ini_set('max_execution_time', 1);
sleep2();
echo "done 2\n";
