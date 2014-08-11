<?php

class z extends domDocument{
	/** variable can have name */
	public $p_array;
	public $p_variable;

	function __construct(){
		$this->p_array[] = 'bonus';
		$this->p_array[] = 'vir';
		$this->p_array[] = 'semper';
		$this->p_array[] = 'tiro';

		$this->p_variable = 'Cessante causa cessat effectus';
	}	
}

$z=new z();
var_dump($z->p_array);
var_dump($z->p_variable);
?>
