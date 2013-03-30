<?php  
$options = array();

class Foo {
    public function __toString() {
        return 'Foo';
    }
}   

function test($options, $queryPart) {
	return ''. (0 ? 1 : $queryPart);
}

var_dump(test($options, new Foo()));
?>