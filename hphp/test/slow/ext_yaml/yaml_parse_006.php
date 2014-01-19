<?php
  var_dump(yaml_parse('---
	"leading tab"
...
'));
  var_dump(yaml_parse('---
  	"space and tab"
...
'));
var_dump(yaml_parse('---
  "key":	"tab before value"
...
'));
var_dump(yaml_parse('---
  "key":  	"space and tab before value"
...
'));
var_dump(yaml_parse('---
-	"tab before value"
...
'));
var_dump(yaml_parse('---
-  	"space and tab before value"
...
'));
?>
