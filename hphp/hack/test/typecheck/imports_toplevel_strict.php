<?hh
require_once(dirname(__FILE__) . '/foo.php');
include_once dirname(__FILE__) . '/foo.php';
require('bar.php');
include('baz.php');
