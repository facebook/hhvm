<?php
var_dump(eval("return <<<\xff\nXYZ\n\xff\n;"));
var_dump(eval("return <<<'\xff'\nXYZ\n\xff\n;"));
