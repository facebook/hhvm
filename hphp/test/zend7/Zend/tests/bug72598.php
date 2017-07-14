<?php
function ref(&$ref) {
	var_dump($ref);
}

new class {
        function __construct() {
                $args = [&$this];
                for ($i = 0; $i < 2; $i++) {
                        $a = array_slice($args, 0, 1);
                        call_user_func_array('ref', $a);
                }
        }
};
?>
