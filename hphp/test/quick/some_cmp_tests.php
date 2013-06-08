<?php

function eq($x, $y) {
  var_dump($x == $y);
}

function lt($x, $y) {
  var_dump($x < $y);
}

function gt($x, $y) {
  var_dump($x > $y);
}

echo "======\n";

eq('Array', array(1,2));
eq('Array', array());
eq(array(), 'Array');
eq(array('a', 'b'), 'Array');
echo "\n";
lt('Array', array(1,2));
lt('Array', array());
lt(array(), 'Array');
lt(array('a', 'b'), 'Array');
echo "\n";
gt('Array', array(1,2));
gt('Array', array());
gt(array(), 'Array');
gt(array('a', 'b'), 'Array');

echo "======\n";

eq('', null);
eq(null, null);
eq(null, '');
eq('', '');
echo "\n";
lt('', null);
lt(null, null);
lt(null, '');
lt('', '');
echo "\n";
gt('', null);
gt(null, null);
gt(null, '');
gt('', '');

echo "======\n";

eq(-1.0, null);
eq(null, -1.0);
echo "\n";
lt(-1.0, null);
lt(null, -1.0);
echo "\n";
gt(-1.0, null);
gt(null, -1.0);
