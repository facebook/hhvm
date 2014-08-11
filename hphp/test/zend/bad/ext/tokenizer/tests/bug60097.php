<?php

var_dump(token_get_all('<?php
<<<DOC1
{$s(<<<DOC2
DOC2
)}
DOC1;
'));

?>
