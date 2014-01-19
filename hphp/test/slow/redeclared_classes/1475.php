<?php

if (true) {
 class base extends Exception {
}
 }
 else {
 class base {
}
 }
class child1 extends base {
}
$obj = new child1;
echo ($obj instanceof Exception) ? "Passed
" : "Failed
";
