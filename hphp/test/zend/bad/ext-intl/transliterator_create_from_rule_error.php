<?php
ini_set("intl.error_level", E_WARNING);

$t = Transliterator::createFromRules();
echo intl_get_error_message(),"\n";

$t = Transliterator::createFromRules("a","b");
echo intl_get_error_message(),"\n";

$t = Transliterator::createFromRules("\x8Fss");
echo intl_get_error_message(),"\n";

$rules = <<<RULES
\`\` } a > “;
\`\` } a > b;
RULES;

$t = Transliterator::createFromRules($rules);
echo intl_get_error_message(),"\n";

$rules = <<<RULES
ffff
RULES;

$t = Transliterator::createFromRules($rules);
echo intl_get_error_message(),"\n";
echo "Done.\n";
