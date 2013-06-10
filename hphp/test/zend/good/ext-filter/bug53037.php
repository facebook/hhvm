<?php
var_dump(
	filter_var("", FILTER_DEFAULT),
    filter_var("", FILTER_DEFAULT, array('flags' => FILTER_FLAG_EMPTY_STRING_NULL))
);
?>