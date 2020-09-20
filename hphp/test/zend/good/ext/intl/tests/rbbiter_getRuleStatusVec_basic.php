<?hh
<<__EntryPoint>>
function entrypoint_rbbiter_getRuleStatusVec_basic(): void {
  ini_set("intl.error_level", E_WARNING);
  ini_set("intl.default_locale", "pt_PT");

  $rules = <<<RULES
\$LN = [[:letter:] [:number:]];
\$S = [.;,:];

!!forward;
\$LN+ {1};
[^.]+ {4};
\$S+ {42};
!!reverse;
\$LN+ {1};
[^.]+ {4};
\$S+ {42};
!!safe_forward;
!!safe_reverse;
RULES;
  $rbbi = new IntlRuleBasedBreakIterator($rules);
  $rbbi->setText('sdfkjsdf88á.... ,;');

  do {
  	var_dump($rbbi->current(), $rbbi->getRuleStatusVec());
  } while ($rbbi->next() != IntlBreakIterator::DONE);
  echo "==DONE==";
}
