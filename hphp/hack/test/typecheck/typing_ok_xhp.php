<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

namespace x {
	class :dumb {}
}

namespace {
	class A implements XHPChild {}

	function foo(mixed $x): void {
		// Almost anything goes in XHP
		$x = <x:dumb>{0}{'hello'}{$x}{new A()}</x:dumb>;
	}
}
