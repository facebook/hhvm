<?php

interface a { }

class_alias('a', 'b');

interface c extends a, b { }

?>