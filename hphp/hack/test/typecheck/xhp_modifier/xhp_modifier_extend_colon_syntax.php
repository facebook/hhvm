//// xhp_modifier_extend_colon_syntax_def.php
<?hh // strict

namespace foo;

xhp class bar {}

//// xhp_modifier_extend_colon_syntax_usage.php
<?hh // strict

namespace user;

xhp class usesfoobar extends :foo:bar {}
