<?php

function output_handler($buffer)
{
	throw new Exception;
}

ob_start('output_handler');

ob_end_clean();
?>