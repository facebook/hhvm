<?hh // strict

namespace NS_expression_statement;

function DoIt(): int { return 20; }

function main(): void {
  $i = 10;	// $i is assigned the value 10; result (10) is discarded
  ++$i;		// $i is incremented; result (11) is discarded
  $i++;		// $i is incremented; result (11) is discarded
  DoIt();		// function DoIt is called; result (return value) is discarded

  $i;		// no side effects; result is discarded, so entirely vacuous
  123;		// likewise ...
  34.5 * 12.6 + 11.987;
  true;

  while ($i-- > 0) {
    ;		// null statement
  }

  $i = 10;
  while ($i-- > 0) {
    continue;	// in this context, equivalent to using a null statement
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();

