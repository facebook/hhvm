<?php
class Foo { function bar() {} }
header_register_callback([new Foo, 'bar']);
header_register_callback([new Foo, 'baz']);
