<?php

	class myClass
	{	
		public $varX;
		public $varY;
	}
	$rc=new reflectionClass('myClass');
	var_dump( $rc->getExtension()) ;
?>
