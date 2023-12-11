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
    $groups[] = ($prefix .= '.'.$results[0]);

    if ($results[1]) {
      $groups[] = ($prefix .= '.'.$results[1]);

      if ($results[2]) {
        $groups[] = ($prefix .= '-'.$results[2]);
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
    $groups[] = ($prefix += HH\Lib\Legacy_FIXME\cast_for_arithmetic($results[0]));

    if ($results[1]) {
      $groups[] = ($prefix += HH\Lib\Legacy_FIXME\cast_for_arithmetic($results[1]));

      if ($results[2]) {
        $groups[] = ($prefix += HH\Lib\Legacy_FIXME\cast_for_arithmetic($results[2]));
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
