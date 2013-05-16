<?php

var_dump(strip_tags('<!-- testing I\'ve been to mars -->foobar'));

var_dump(strip_tags('<a alt="foobar">foo<!-- foo! --></a>bar'));

var_dump(strip_tags('<a alt="foobar"/>foo<?= foo! /* <!-- "cool" --> */ ?>bar'));

var_dump(strip_tags('< ax'));

var_dump(strip_tags('<! a>'));

var_dump(strip_tags('<? ax'));

?>