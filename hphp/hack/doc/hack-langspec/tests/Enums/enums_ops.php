<?hh // strict

namespace NS_enum_ops;

enum Status: int {
  Stopped = 0;
  Stopping = 1;
  Starting = 2;
  Started = 3;
}

enum State: string {
  Ready = 'READY';
  Set = 'SET';
  Go = 'GO';
  MT = '';
}

function main(): void {
  opsOnEnums(Status::Stopping, Status::Stopping, Status::Starting, Status::Stopped,
    State::Set, State::Set, State::Go, State::MT);
}

function opsOnEnums(
    Status $enumI1, Status $enumI2, Status $enumI3, Status $enumI4,
    State $enumS1, State $enumS2, State $enumS3, State $enumS4
  ): void {

  echo "\n====== various operations on int and string enums  ======\n\n";
/*
  echo "______ \$enumI1\n"; var_dump($enumI1);
  echo "______ \$enumI3\n"; var_dump($enumI3);
  echo "______ \$enumS1\n"; var_dump($enumS1);
  echo "______ \$enumS3\n"; var_dump($enumS3);
*/

//  $enumI1++;

//  --$enumI1;
/*
  echo "\n______ !\n"; var_dump(!$enumI1);	// bool(false)	!1
  echo "______ !\n"; var_dump(!$enumI3);	// bool(false)	!2
  echo "______ !\n"; var_dump(!$enumS1);	// bool(false)	!'SET'
  echo "______ !\n"; var_dump(!$enumS3);	// bool(false)	!'GO'
  echo "______ !\n"; var_dump(!$enumI4);	// bool(true)	!0
  echo "______ !\n"; var_dump(!$enumS4);	// bool(true)	!''
*/

//  +$enumI1;
//  -$enumI1;
//  ~$enumI1;

//  $enumI1 * 1;
//  $enumI1 / 2;
//  $enumI1 % 2;

//  $enumI1 + 1;
//  $enumI1 - 1;

/*
  echo "\n______ .\n"; var_dump($enumI1 . 1);	// string(2) "11"
  echo "______ .\n"; var_dump($enumI3 . 1);	// string(2) "21"
  echo "______ .\n"; var_dump($enumS1 . 1);	// string(4) "SET1"
  echo "______ .\n"; var_dump($enumS3 . 1);	// string(3) "GO1"

//  $enumI1 << 2;
//  2000 >> $enumI1;
*/

///*
  echo "\n______ <\n"; var_dump($enumI1 < $enumI3);	// bool(true)	1 < 2
  echo "______ >=\n"; var_dump($enumI1 >= $enumI3);	// bool(false)	1 >= 2

  echo "\n______ <\n"; var_dump($enumS1 < $enumS3);	// bool(false)	'SET' < 'GO'
  echo "______ >=\n"; var_dump($enumS1 >= $enumS3);	// bool(true)	'SET' >= 'GO'

  echo "\n______ ==\n"; var_dump($enumI1 == $enumI3);	// bool(false)
  echo "______ !==\n"; var_dump($enumI1 !== $enumI3);	// bool(true)

  echo "\n______ ==\n"; var_dump($enumS1 == $enumS3);	// bool(false)
  echo "______ !==\n"; var_dump($enumS1 !== $enumS3);	// bool(true)
//*/

/*
//  $enumI1 & 8;
*/

/*
  echo "\n______ &&\n"; var_dump($enumI1 && $enumI3);	// 1 && 2 => bool(true)
  echo "______ &&\n"; var_dump($enumI4 && $enumI1);	// 0 && 1 => bool(false)
  echo "______ ||\n"; var_dump($enumI4 || $enumI1);	// 0 || 1 => bool(true)

  echo "\n______ && mixed\n"; var_dump($enumI1 && $enumS3);	// 1 && 'Go' => bool(true)

  echo "\n______ &&\n"; var_dump($enumS1 && $enumS3);	// 'Ready' && 'Go' => bool(true)
  echo "______ &&\n"; var_dump($enumS4 && $enumS1);	// '' && 'Ready' => bool(false)
  echo "______ ||\n"; var_dump($enumS4 || $enumS1);	// '' || 'Ready' => bool(true)

  echo "\n______ ?:\n"; var_dump($v = $enumI1 ? $enumI1 : $enumI3);	// int(1)
  echo "______ ?:\n"; var_dump($v = $enumI4 ? $enumI1 : $enumI3);	// int(2)
  echo "______ ?:\n"; var_dump($v = $enumS1 ? $enumS1 : $enumS3);	// string(3) 'SET'
  echo "______ ?:\n"; var_dump($v = $enumS4 ? $enumS1 : $enumS3);	// string(2) 'Go'
*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
