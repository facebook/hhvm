<?hh
<<__EntryPoint>>
function entrypoint_rbbiter_getBinaryRules_basic(): void {
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
  $rbbi->setText("sdfkjsdf88\xc3\xa1.... ,;");

  $br = $rbbi->getBinaryRules();

  $rbbi2 = new IntlRuleBasedBreakIterator($br, true);

  var_dump($rbbi->getRules(), $rbbi2->getRules());
  var_dump($rbbi->getRules() == $rbbi2->getRules());
  echo "==DONE==";
}
