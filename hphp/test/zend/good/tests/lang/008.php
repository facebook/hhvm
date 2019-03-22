<?php

abstract final class TestStatics {
  public static $a =1;
}

function Test()
{
	echo TestStatics::$a . " ";
	TestStatics::$a++;
	if(TestStatics::$a<10): Test(); endif;
}

Test();

