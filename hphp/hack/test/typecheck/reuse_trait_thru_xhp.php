<?hh // strict
/**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

namespace {
	trait A {}
	class Base {
		use A;
	}
}

namespace foo {
	class :core {
		use \A;
	}
}

namespace {
	class :foo extends Base {
		attribute :foo:core;
	}
}
