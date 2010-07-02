<?php
  require("taint10_aux.php");
  $aaa = foo();
  echo $aaa; // tainted
