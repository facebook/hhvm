<?php
  $arr = array(
	"This is string one.",
	"This is string two.",
	array(
		"This is another string.",
		"This is a last string."),
	"This is a last string.");

  echo serialize(str_replace("string", "strung", $arr)) . "\n";
  echo serialize(str_replace("string", "strung", $arr)) . "\n";
  echo serialize(str_replace(" ", "", $arr)) . "\n";
  echo serialize(str_replace(" ", "", $arr)) . "\n";
?>