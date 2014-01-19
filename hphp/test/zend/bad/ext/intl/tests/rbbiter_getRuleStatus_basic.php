<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$rules = <<<RULES
\$LN = [[:letter:] [:number:]];
\$S = [.;,:];

!!forward;
\$LN+ {1};
\$S+ {42};
!!reverse;
\$LN+ {1};
\$S+ {42};
!!safe_forward;
!!safe_reverse;
RULES;
$rbbi = new IntlRuleBasedBreakIterator($rules);
$rbbi->setText('sdfkjsdf88á.... ,;');

do {
	echo "pos        : {$rbbi->current()}\n",
		 "rule status: {$rbbi->getRuleStatus()}\n";
} while ($rbbi->next() != IntlBreakIterator::DONE);

?>
==DONE==