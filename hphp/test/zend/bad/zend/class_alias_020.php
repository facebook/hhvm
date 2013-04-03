<?php

namespace foo;


class foo {
}

class_alias(__NAMESPACE__ .'\foo', 'foo');

namespace foo\bar;

class foo { 
}

class_alias(__NAMESPACE__ .'\foo', 'bar');


var_dump(new \foo, new \bar);

var_dump(new \foo\foo, new \foo\bar);

?>