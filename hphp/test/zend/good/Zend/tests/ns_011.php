<?php
namespace test\ns1;

function foo() {
  echo __FUNCTION__,"\n";
}
  
foo();
\test\ns1\foo();
bar();
\test\ns1\bar();

function bar() {
  echo __FUNCTION__,"\n";
}
