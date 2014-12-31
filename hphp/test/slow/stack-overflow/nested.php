<?php


function foo($x) {
  bar(

    foo($x)

  );
}

function bar($x) {
  var_dump(__METHOD__);
}

foo(0);
