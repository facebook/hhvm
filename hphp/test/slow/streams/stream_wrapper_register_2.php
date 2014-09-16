<?php

class B {

}

stream_wrapper_register("cde", "B") or die("Failed to register protocol");

touch('cde://g');
