<?hh // strict

namespace NS_shape_subtyping;

enum Bank: int {
  INVALID = 0;
  DEPOSIT = 1;
  WITHDRAWAL = 2;
  TRANSFER = 3;
}

type Transaction = shape('trtype' => Bank);
type Deposit = shape('trtype' => Bank, 'toaccnum' => int, 'amount' => float);
type Withdrawal = shape('trtype' => Bank, 'fromaccnum' => int, 'amount' => float);
type Transfer = shape('trtype' => Bank, 'fromaccnum' => int, 'toaccnum' => int, 'amount' => float);

function main(): void {
  processTransaction(shape('trtype' => Bank::DEPOSIT, 'toaccnum' => 23456, 'amount' => 100.00));
  processTransaction(shape('trtype' => Bank::WITHDRAWAL, 'fromaccnum' => 3157, 'amount' => 100.00));
  processTransaction(shape('trtype' => Bank::TRANSFER, 'fromaccnum' => 23456, 'toaccnum' => 3157, 'amount' => 100.00));
}

function processTransaction(Transaction $t): void {
  var_dump($t);

  $ary = Shapes::toArray($t);

  switch ($t['trtype']) {
  case Bank::TRANSFER:
    echo "Transfer: " . ((string)$ary['amount']) . " from Account " . ((string)$ary['fromaccnum'])
      . " to Account " . ((string)$ary['toaccnum']) . "\n";
    break;

  case Bank::DEPOSIT:
    echo "Deposit: " . ((string)$ary['amount']) . " to Account " . ((string)$ary['toaccnum']) . "\n";
    break;

  case Bank::WITHDRAWAL:
    echo "Withdrawal: " . ((string)$ary['amount']) . " from Account " . ((string)$ary['fromaccnum']) . "\n";
    break;

  default:
    break;
  }
}

/* HH_FIXME[1002] call to main in strict*/
main();