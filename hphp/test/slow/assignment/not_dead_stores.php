<?hh

function getTestArray() :mixed{
  return vec['opera', '12', '16'];
}

function stringFunc() :mixed{
  $results = getTestArray();
  var_dump($results);
  if (!$results) return null;

  $groups = vec[];
  $prefix = 'foo.browser';

  if ($results[0]) {
    $prefix .= '.'.$results[0]; $groups[] = $prefix;

    if ($results[1]) {
      $prefix .= '.'.$results[1]; $groups[] = $prefix;

      if ($results[2]) {
        $prefix .= '-'.$results[2]; $groups[] = $prefix;
      }
    }
  }

  return $groups;
}

function intFunc() :mixed{
  $results = getTestArray();
  var_dump($results);
  if (!$results) return null;

  $groups = vec[];
  $prefix = 1;

  if ($results[0]) {
    $prefix += HH\Lib\Legacy_FIXME\cast_for_arithmetic($results[0]); $groups[] = $prefix;

    if ($results[1]) {
      $prefix += HH\Lib\Legacy_FIXME\cast_for_arithmetic($results[1]); $groups[] = $prefix;

      if ($results[2]) {
        $prefix += HH\Lib\Legacy_FIXME\cast_for_arithmetic($results[2]); $groups[] = $prefix;
      }
    }
  }

  return $groups;
}


<<__EntryPoint>>
function main_not_dead_stores() :mixed{
var_dump(stringFunc());
var_dump(intFunc());
}
