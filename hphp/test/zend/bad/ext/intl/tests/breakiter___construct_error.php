<?php
ini_set("intl.error_level", E_WARNING);

//missing ; at the end:
var_dump(new IntlRuleBasedBreakIterator('[\p{Letter}\uFFFD]+;[:number:]+'));
var_dump(new IntlRuleBasedBreakIterator());
var_dump(new IntlRuleBasedBreakIterator(1,2,3));
var_dump(new IntlRuleBasedBreakIterator('[\p{Letter}\uFFFD]+;[:number:]+;', array()));
var_dump(new IntlRuleBasedBreakIterator('[\p{Letter}\uFFFD]+;[:number:]+;', true));
