<?php
	$������ = array('������' => '������');

	class �berK��li�� 
	{
		public $��������ber = '������';
	}
  
    $foo = new �berk��li��();
  
	var_dump(serialize($foo));
	var_dump(unserialize(serialize($foo)));
	var_dump(serialize($������));
	var_dump(unserialize(serialize($������)));
?>
