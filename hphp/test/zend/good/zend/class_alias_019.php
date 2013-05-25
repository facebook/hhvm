<?php

namespace foo;


class foo {
}

class_alias(__NAMESPACE__ .'\foo', 'foo');
class_alias('\foo', 'foo');

?>