<?hh

$str = <<<EOT
foo
EOT;

function_call(<<<EOTA
foo
EOTA
);

function_call(<<<EOTB
foo
EOTB
,
<<<EOTC
bar
EOTC
);

$str = <<<'EOTD'
foo
EOTD;

function_call(<<<'EOTE'
foo
EOTE
);

function_call(<<<'EOTF'
foo
EOTF
,
<<<'EOTG'
bar
EOTG
);

function foo() {
  $bar = "bar";
  return JSON::decode(<<<JSON
  { "foo": "{$bar}" }
JSON
  );
}

$csv = <<<CSV
2017-04-07,$some_variable,frankenstein,dracula,chocula,wolverine,http://www.longlines.com;
2017-04-07,2,frankenstein,dracula,chocula,wolverine,http://www.longlines.com;
2017-04-07,{$some_variable},frankenstein,dracula,chocula,wolverine,http://www.longlines.com;
2017-04-07,2,frankenstein,dracula,chocula,wolverine,http://www.longlines.com;
2017-04-07,$object->member,frankenstein,dracula,chocula,wolverine,http://www.longlines.com;
2017-04-07,{$some_variable[$index1 + $index2]},frankenstein,dracula,chocula,wolverine,http://www.longlines.com;
CSV;
