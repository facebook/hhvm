<?php
namespace ns;
const ok = 0;
class foo {
	const ok = 0;
}
var_dump(defined('ns\\ok'));
var_dump(defined('ns\\bug'));
var_dump(defined('\\ns\\ok'));
var_dump(defined('\\ns\\bug'));
var_dump(defined('ns\\foo::ok'));
var_dump(defined('ns\\foo::bug'));
var_dump(defined('\\ns\\foo::ok'));
var_dump(defined('\\ns\\foo::bug'));
var_dump(defined('ns\\bar::bug'));
var_dump(defined('\\ns\\bar::bug'));