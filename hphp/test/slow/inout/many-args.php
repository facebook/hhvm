<?hh

function inoutArgument31(
  int $arg1,  int $arg2,  int $arg3,  int $arg4,  int $arg5,  int $arg6,  int $arg7,  int $arg8,  int $arg9,  int $arg10,
  int $arg11, int $arg12, int $arg13, int $arg14, int $arg15, int $arg16, int $arg17, int $arg18, int $arg19, int $arg20,
  int $arg21, int $arg22, int $arg23, int $arg24, int $arg25, int $arg26, int $arg27, int $arg28, int $arg29, int $arg30,
  inout int $arg31, int $arg32, int $arg33, int $arg34, int $arg35, int $arg36, int $arg37, int $arg38, int $arg39, int $arg40,
  int $arg41, int $arg42, int $arg43, int $arg44, int $arg45, int $arg46, int $arg47, int $arg48, int $arg49, int $arg50,
  int $arg51, int $arg52, int $arg53, int $arg54, int $arg55, int $arg56, int $arg57, int $arg58, int $arg59, int $arg60,
  int $arg61, int $arg62, int $arg63, int $arg64, int $arg65, int $arg66, int $arg67, int $arg68, int $arg69, int $arg70,
) :mixed{
  $arg31 = 31;
}

function inoutArgument31Last(
  int $arg1,  int $arg2,  int $arg3,  int $arg4,  int $arg5,  int $arg6,  int $arg7,  int $arg8,  int $arg9,  int $arg10,
  int $arg11, int $arg12, int $arg13, int $arg14, int $arg15, int $arg16, int $arg17, int $arg18, int $arg19, int $arg20,
  int $arg21, int $arg22, int $arg23, int $arg24, int $arg25, int $arg26, int $arg27, int $arg28, int $arg29, int $arg30,
  inout int $arg31,
) :mixed{
  $arg31 = 31;
}

function inoutArgument32(
  int $arg1,  int $arg2,  int $arg3,  int $arg4,  int $arg5,  int $arg6,  int $arg7,  int $arg8,  int $arg9,  int $arg10,
  int $arg11, int $arg12, int $arg13, int $arg14, int $arg15, int $arg16, int $arg17, int $arg18, int $arg19, int $arg20,
  int $arg21, int $arg22, int $arg23, int $arg24, int $arg25, int $arg26, int $arg27, int $arg28, int $arg29, int $arg30,
  int $arg31, inout int $arg32, int $arg33, int $arg34, int $arg35, int $arg36, int $arg37, int $arg38, int $arg39, int $arg40,
  int $arg41, int $arg42, int $arg43, int $arg44, int $arg45, int $arg46, int $arg47, int $arg48, int $arg49, int $arg50,
  int $arg51, int $arg52, int $arg53, int $arg54, int $arg55, int $arg56, int $arg57, int $arg58, int $arg59, int $arg60,
  int $arg61, int $arg62, int $arg63, int $arg64, int $arg65, int $arg66, int $arg67, int $arg68, int $arg69, int $arg70,
) :mixed{
  $arg32 = 32;
}

function inoutArgument32Last(
  int $arg1,  int $arg2,  int $arg3,  int $arg4,  int $arg5,  int $arg6,  int $arg7,  int $arg8,  int $arg9,  int $arg10,
  int $arg11, int $arg12, int $arg13, int $arg14, int $arg15, int $arg16, int $arg17, int $arg18, int $arg19, int $arg20,
  int $arg21, int $arg22, int $arg23, int $arg24, int $arg25, int $arg26, int $arg27, int $arg28, int $arg29, int $arg30,
  int $arg31, inout int $arg32,
) :mixed{
  $arg32 = 32;
}


function inoutArgument70(
  int $arg1,  int $arg2,  int $arg3,  int $arg4,  int $arg5,  int $arg6,  int $arg7,  int $arg8,  int $arg9,  int $arg10,
  int $arg11, int $arg12, int $arg13, int $arg14, int $arg15, int $arg16, int $arg17, int $arg18, int $arg19, int $arg20,
  int $arg21, int $arg22, int $arg23, int $arg24, int $arg25, int $arg26, int $arg27, int $arg28, int $arg29, int $arg30,
  int $arg31, int $arg32, int $arg33, int $arg34, int $arg35, int $arg36, int $arg37, int $arg38, int $arg39, int $arg40,
  int $arg41, int $arg42, int $arg43, int $arg44, int $arg45, int $arg46, int $arg47, int $arg48, int $arg49, int $arg50,
  int $arg51, int $arg52, int $arg53, int $arg54, int $arg55, int $arg56, int $arg57, int $arg58, int $arg59, int $arg60,
  int $arg61, int $arg62, int $arg63, int $arg64, int $arg65, int $arg66, int $arg67, int $arg68, int $arg69, inout int $arg70,
) :mixed{
  $arg70 = 70;
}

<<__EntryPoint>>
function main(): void {
  $arg = 1;
  inoutArgument31(
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    inout $arg, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  );
  var_dump($arg);

  $arg = 1;
  inoutArgument31Last(
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    inout $arg,
  );
  var_dump($arg);

  $arg = 1;
  inoutArgument32(
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, inout $arg, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  );
  var_dump($arg);

  $arg = 1;
  inoutArgument32Last(
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, inout $arg,
  );
  var_dump($arg);

  $arg = 1;
  inoutArgument70(
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, inout $arg,
  );
  var_dump($arg);

  try {
    inoutArgument31(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument31(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument31Last(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument31Last(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument32(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument32(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument32Last(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument32Last(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument70(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    inoutArgument70(
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1,
    );
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
