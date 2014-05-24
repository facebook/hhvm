<?php
set_include_path(__DIR__);
spl_autoload_register();
var_dump(new ExampleClass());
var_dump(new Namespaced\Foo());
var_dump(new Namespaced\HerpDerp());
var_dump(new Namespaced\Nested\Ponies());
