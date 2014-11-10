<?php

class B {

}

stream_wrapper_register("cde", "B") or die("Failed to register protocol");

var_dump(touch('cde://a'));
var_dump(chmod('cde://b', 0123));
var_dump(chown('cde://c', 'user'));
var_dump(chown('cde://d', 123));
var_dump(chgrp('cde://e', 'user'));
var_dump(chgrp('cde://f', 123));
