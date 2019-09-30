//// xhp_namespace_use_in_same_namespace_def.php
<?hh // strict

namespace foo;

class :bar {}
//// xhp_namespace_use_in_same_namespace_usage.php
<?hh // strict

namespace foo;

function test_def(): void {
	<bar />;
}
