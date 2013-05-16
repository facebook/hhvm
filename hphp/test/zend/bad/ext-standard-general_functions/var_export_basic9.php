<?php
	$x = new stdClass();
	$x->{'\'\\'} = 7;
	echo var_export($x);