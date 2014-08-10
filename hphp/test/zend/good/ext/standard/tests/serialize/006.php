<?php
	$åäöÅÄÖ = array('åäöÅÄÖ' => 'åäöÅÄÖ');

	class ÜberKööliäå 
	{
		public $åäöÅÄÖüÜber = 'åäöÅÄÖ';
	}
  
    $foo = new Überkööliäå();
  
	var_dump(serialize($foo));
	var_dump(unserialize(serialize($foo)));
	var_dump(serialize($åäöÅÄÖ));
	var_dump(unserialize(serialize($åäöÅÄÖ)));
?>
