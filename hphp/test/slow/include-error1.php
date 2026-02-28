<?hh

<<__EntryPoint>>
function main_include_error1() :mixed{
include('non-existing-file.php');
include 'non-existing-file.php';
include_once('non-existing-file.php');
include_once 'non-existing-file.php';
require('non-existing-file.php');
}
