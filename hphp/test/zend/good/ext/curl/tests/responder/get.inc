<?hh
  $test = isset(\HH\global_get('_GET')['test']) ? \HH\global_get('_GET')['test'] : null;
  switch($test) {
    case 'post':
      var_dump(\HH\global_get('_POST'));
      break;
    case 'getpost':
      var_dump(\HH\global_get('_GET'));
      var_dump(\HH\global_get('_POST'));
      break;
    case 'referer':
      echo \HH\global_get('_SERVER')['HTTP_REFERER'];
      break;
    case 'useragent':
      echo \HH\global_get('_SERVER')['HTTP_USER_AGENT'];
      break;
    case 'httpversion':
      echo \HH\global_get('_SERVER')['SERVER_PROTOCOL'];
      break;
    case 'cookie':
      echo \HH\global_get('_SERVER')['foo'];
      break;
    case 'encoding':
      echo \HH\global_get('_SERVER')['HTTP_ACCEPT_ENCODING'];
      break;
    case 'contenttype':
      header('Content-Type: text/plain;charset=utf-8');
      break;
    case 'file':
      if (isset(_FILES['file'])) {
          echo \HH\global_get('_FILES')['file']['name'] . '|' . _FILES['file']['type'];
      }
      break;
    case 'method':
      echo \HH\global_get('_SERVER')['REQUEST_METHOD'];
      break;
    default:
      echo "Hello World!\n";
      echo "Hello World!";
      break;
  }
