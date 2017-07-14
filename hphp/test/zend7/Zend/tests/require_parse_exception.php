<?php

function test_parse_error($code) {
    try {
        require 'data://text/plain;base64,' . base64_encode($code);
    } catch (ParseError $e) {
        echo $e->getMessage(), " on line ", $e->getLine(), "\n";
    }
}

test_parse_error(<<<'EOC'
<?php
{ { { { { }
EOC
);

test_parse_error(<<<'EOC'
<?php
/** doc comment */
function f() {
EOC
);

test_parse_error(<<<'EOC'
<?php
empty
EOC
);

test_parse_error('<?php
var_dump(078);');

test_parse_error('<?php
var_dump("\u{xyz}");');
test_parse_error('<?php
var_dump("\u{ffffff}");');

?>
