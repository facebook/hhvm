<?php

namespace {
	echo __COMPILER_HALT_OFFSET__, "\n";
	echo \__COMPILER_HALT_OFFSET__, "\n";
}

namespace Foo {
	echo __COMPILER_HALT_OFFSET__, "\n";
	echo \__COMPILER_HALT_OFFSET__, "\n";
	echo namespace\__COMPILER_HALT_OFFSET__, "\n";

}

__halt_compiler();

?>
