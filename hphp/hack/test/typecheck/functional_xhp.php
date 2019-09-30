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
	class Blah {}

	class :xx extends Blah {
		attribute Map<string, int> k;
	}

	class :xy {}
}

namespace {
	class A {}

	function test(): void {
		$x = <x:xx k={Map {}}></x:xx>;
	}
}
