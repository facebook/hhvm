//// xhp_namespace_use_outside_namespace_1_def.php
<?hh // strict

namespace foo;

class :bar {}
//// xhp_namespace_use_outside_namespace_1_usage.php
<?hh // strict

function test_def(): void {
	<foo:bar />;
}
