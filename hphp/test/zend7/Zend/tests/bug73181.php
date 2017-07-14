<?php

function x() {
    parse_str("1&x");
    var_dump(get_defined_vars());
}

x();

?>
