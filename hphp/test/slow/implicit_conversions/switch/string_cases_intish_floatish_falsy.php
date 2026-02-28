<?hh

<<__EntryPoint>>
function main() :mixed{
  f1("");
  f1(null);
  f1(false);
  f1("0");
  f1("0eab");
  f1("0.0");
  f1(0.0);
  f1(0);
  f1(true);
  f1(false);
  f1("4abc");
  f1(4);
  f1("4.0");
  f1(new Derp());

  f2(1);
  f2(2);
  f2(2.0);
  f2(true);
  f2(false);
  f2(null);
  f2(vec[]);
  f2(3.21200);
  f2("");
  f2("0");
  f2("0eab");
  f2("0.0");
  f1(new Derp());

  g(0);
  g(null);
  g(false);
  g(true);

  h("3");
  h("3abc");
  h("3a");
  h(3);
  h(3.0);
}

class Derp {}

function f1($x) :mixed{
  var_dump($x);
  print "goes to:\n";
  switch ($x) {
    case "123":
      print '"123"';
      break;
    case "4abc":
      print '"4abc"';
      break;
    case "0":
      print '"0"';
      break;
    case "":
      print '""';
      break;
    case "Derp":
      print '"Derp"';
      break;
    default:
      print "default";
      break;
  }
  echo "\n";
}

function f2($x) :mixed{
  var_dump($x);
  print "goes to:\n";
  switch ($x) {
    case "foo":
      print "foo";
      break;
    case "1":
      print "1";
      break;
    case "2.0":
      print "2.0";
      break;
    case "2ab":
      print "2ab";
      break;
    case "3.212":
      print "3.212";
      break;
    case "0":
      print "0";
      break;
    case "":
      print "{empty str}";
      break;
    default:
      print "default";
      break;
  }
  echo "\n";
}

function g($x) :mixed{
  var_dump($x);
  print "goes to:\n";
  switch ($x) {
    case "":
      print "{empty str}";
      break;
    case "0":
      print "0";
      break;
    default:
      print "default";
      break;
  }
  echo "\n";
}

function h($x) :mixed{
  var_dump($x);
  print "goes to:\n";
  switch ($x) {
    case "3.0":
      print "3.0";
      break;
    case "3.0abc":
      print "3.0abc";
      break;
    case "3":
      print "3";
      break;
    default:
      print "";
  }
  echo "\n";
}
