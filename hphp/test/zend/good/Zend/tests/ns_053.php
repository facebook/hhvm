<?php
namespace test\ns1;

define(__NAMESPACE__ . '\\NAME', basename(__FILE__));
echo NAME."\n";
echo \test\ns1\NAME."\n";