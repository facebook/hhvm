<?php

interface A {function a();}
interface B extends A {}
var_dump(method_exists('B', 'a'));
