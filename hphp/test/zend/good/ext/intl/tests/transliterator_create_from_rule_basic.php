<?hh
<<__EntryPoint>>
function entrypoint_transliterator_create_from_rule_basic(): void {
  ini_set("intl.error_level", E_WARNING);

  $rules = <<<RULES
α <> y;
\`\` } a > “;
RULES;

  $t = Transliterator::createFromRules($rules);
  echo $t->id,"\n";

  echo $t->transliterate("``akk ``bkk ``aooy"),"\n";

  $u = transliterator_create_from_rules($rules, Transliterator::REVERSE);

  echo $u->transliterate("``akk ``bkk ``aooy"), "\n";

  echo "Done.\n";
}
