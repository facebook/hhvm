<?php

var_dump(preg_replace('', array(), ''));

var_dump(preg_replace(array('/\da(.)/ui', '@..@'), '$1', '12Abc'));
var_dump(preg_replace(array('/\da(.)/ui', '@(.)@'), '$1', array('x','a2aA', '1av2Ab')));


var_dump(preg_replace(array('/[\w]+/'), array('$'), array('xyz', 'bdbd')));
var_dump(preg_replace(array('/\s+/', '~[b-d]~'), array('$'), array('x y', 'bd bc')));

echo "==done==\n";

?>