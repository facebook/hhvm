<?php
var_dump(yaml_emit(null));
var_dump(yaml_emit(true));
var_dump(yaml_emit(false));
var_dump(yaml_emit(10));
var_dump(yaml_emit(-10));
var_dump(yaml_emit(123.456));
var_dump(yaml_emit(-123.456));
var_dump(yaml_emit("yes"));
var_dump(yaml_emit("no"));
var_dump(yaml_emit("~"));
var_dump(yaml_emit("-"));
var_dump(yaml_emit("'"));
var_dump(yaml_emit('"'));
var_dump(yaml_emit("I\\xF1t\\xEBrn\\xE2ti\\xF4n\\xE0liz\\xE6ti\\xF8n"));
var_dump(yaml_emit("# looks like a comment"));
var_dump(yaml_emit("@looks_like_a_ref"));
var_dump(yaml_emit("&looks_like_a_alias"));
var_dump(yaml_emit("!!str"));
var_dump(yaml_emit("%TAG ! tag:looks.like.one,999:"));
var_dump(yaml_emit("!something"));
var_dump(yaml_emit("Hello world!"));
var_dump(yaml_emit("This is a string with\nan embedded newline."));
$str = <<<EOD
This string was made with a here doc.

It contains embedded newlines.
  		It also has some embedded tabs.

Here are some symbols:
`~!@#$%^&*()_-+={}[]|\:";'<>,.?/

These are extended characters: Iñtërnâtiônàlizætiøn


EOD;
var_dump(yaml_emit($str));
?>