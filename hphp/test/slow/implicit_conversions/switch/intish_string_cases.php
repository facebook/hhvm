<?hh

<<__EntryPoint>>
function main() {
  a1('4.0');
  a2('4');
  a2(4);
  a3(false);
  a4('baz');
}

function a1($x) {
  switch ($x) {
    case '5':
      print '5';
      break;
    case '4':
      print '4';
      break;
    case '0':
      print '0';
      break;
    case '3':
      print '3';
      break;
    case '8':
      print '8';
      break;
    case '16':
      print '16';
      break;
    case '32':
      print '32';
      break;
    case '64':
      print '64';
      break;
    default:
      print 'default';
      break;
  }
}


function a2($x) {
  switch ($x) {
    case '5':
      print '5';
      break;
    case '4':
      print '4';
      break;
    case '0':
      print '0';
      break;
    case '3':
      print '3';
      break;
    case '8':
      print '8';
      break;
    case '16':
      print '16';
      break;
    case '32':
      print '32';
      break;
    case '64':
      print '64';
      break;
    default:
      print 'default';
      break;
  }
}

function a3($x) {
  switch ($x) {
    case '5':
      print '5';
      break;
    case '4':
      print '4';
      break;
    case '0':
      print '0';
      break;
    case '3':
      print '3';
      break;
    case '8':
      print '8';
      break;
    case '16':
      print '16';
      break;
    case '32':
      print '32';
      break;
    case '64':
      print '64';
      break;
    default:
      print 'default';
      break;
  }
}

function a4($x) {
  switch ($x) {
    case 'foo':
      print 'foo';
      break;
    case 'bar':
      print 'bar';
      break;
    case 'baz':
      print 'baz';
      break;
    case 'foo2':
      print 'foo2';
      break;
    case 'bar2':
      print 'bar2';
      break;
    case 'baz2':
      print 'baz2';
      break;
    case 'foo3':
      print 'foo3';
      break;
    case 'bar3':
      print 'bar3';
      break;
    case 'baz3':
      print 'baz3';
      break;
    default:
      print 'default';
      break;
  }
}
