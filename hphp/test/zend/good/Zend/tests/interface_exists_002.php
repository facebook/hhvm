<?php

namespace foo;

interface IFoo { }

interface ITest extends IFoo { }

interface IBar extends IFoo { }


var_dump(interface_exists('IFoo'));
var_dump(interface_exists('foo\\IFoo'));
var_dump(interface_exists('FOO\\ITEST'));

?>