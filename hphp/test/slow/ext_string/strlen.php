<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class StrlenTest {
  public function __toString()[] :mixed{
    return 'StrlenTest__toString!';
  }
}

<<__EntryPoint>>
function main(): void {
  set_error_handler(($errno, $errstr, ...) ==> {
    throw new Exception($errstr);
  });

  try { var_dump(strlen()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  try { var_dump(strlen(null)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(true)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(false)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  try { var_dump(strlen(123456)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(123456.0)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(123.456)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  try { var_dump(strlen(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(vec["str"])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  try { var_dump(strlen(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(dict[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(keyset[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  try { var_dump(strlen(new stdClass())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump(strlen(new StrlenTest())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

  var_dump(strlen("null"));
  var_dump(strlen("true"));
  var_dump(strlen("false"));
  var_dump(strlen("123456"));
  var_dump(strlen("123456.0"));
  var_dump(strlen("123.456"));
  var_dump(strlen("array()"));
  var_dump(strlen("array('str')"));
  var_dump(strlen("new stdClass()"));
}
