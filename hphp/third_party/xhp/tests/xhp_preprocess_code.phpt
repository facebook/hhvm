--TEST--
xhp_preprocess_code
--FILE--
<?php
$xhp = <<<XHP
<?php
class :thing {
}
XHP;
print_r(xhp_preprocess_code($xhp));
--EXPECT--
Array
(
    [new_code] => <?php
class xhp_thing{
}
)
