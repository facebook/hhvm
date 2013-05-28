<?php

function one() {
 echo 'one';
}
fb_rename_function('one', 'two');
fb_rename_function('two', 'three');
three();
