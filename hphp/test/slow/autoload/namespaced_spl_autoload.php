<?php
set_include_path(__DIR__);
spl_autoload_extensions('.inc');
spl_autoload('Namespaced\Foo');
spl_autoload('Namespaced\HerpDerp');
spl_autoload('Namespaced\Nested\Ponies');

var_dump(new Namespaced\Foo());
var_dump(new Namespaced\HerpDerp());
var_dump(new Namespaced\Nested\Ponies());
