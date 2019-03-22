<?php
class asserter {
	public function call($function) {
	}
}

$asserter = new asserter();
$function = 'md5';

$closure = function() use ($asserter, $function) {
        $asserter->call($function);
};

$closure();

var_dump($function);

$closure = function() use ($asserter, $function) {
        $asserter->call($function);
};

$closure();

var_dump($function);

$closure = function() use ($asserter, $function) {
        $asserter->call($function);
};

$closure();

var_dump($function);
