<?hh
/* Prototype: bool symlink ( string $target, string $link );
   Description: creates a symbolic link to the existing target with the specified name link

   Prototype: bool is_link ( string $filename );
   Description: Tells whether the given file is a symbolic link.

   Prototype: bool link ( string $target, string $link );
   Description: Create a hard link

   Prototype: int linkinfo ( string $path );
   Description: Gets information about a link
*/

/* Variation 1 : Creating links across directories where linkname is stored as an object and array member */
class object_temp {
  public $linkname;
  function __construct($link) {
    $this->linkname = $link;
  }
}

<<__EntryPoint>>
function main(): void {
  // creating temp directory which will contain temp file and links created
  $dirname = sys_get_temp_dir().'/'.'symlink_link_linkinfo_is_link_variation1/test/home';
  mkdir($dirname, 0777, true);

  // creating temp file; links are created to this file later on
  $filename = sys_get_temp_dir().'/'.'symlink_link_linkinfo_is_link_variation1/symlink_link_linkinfo_is_link_variation1.tmp';
  $fp = fopen($filename, "w");
  fclose($fp);

  echo "*** Testing symlink(), link(), linkinfo() and is_link() with linknames stored as members in an object ***\n";
  $obj = new object_temp("$dirname/symlink_link_linkinfo_is_link_link.tmp");
  /* Testing on soft links */
  echo "\n-- Working with soft links --\n";
  // creating soft link
  var_dump(symlink($filename, $obj->linkname));
  // check if the link exists
  var_dump(linkinfo($obj->linkname));
  // check if link is soft link
  var_dump(is_link($obj->linkname));
  // delete the link created
  unlink($obj->linkname);
  // clear the cache
  clearstatcache();

  /* Testing on hard links */
  echo "\n-- Working with hard links --\n";
  // creating hard link
  var_dump(link($filename, $obj->linkname));
  // check if the link exists
  var_dump(linkinfo($obj->linkname));
  // check if link is soft link; expected: false as the link is a hardlink
  var_dump(is_link($obj->linkname));
  // delete the link created
  unlink($obj->linkname);
  // clear the cache
  clearstatcache();

  echo "\n*** Testing symlink(), link(), linkinfo() and is_link() with linknames stored as members of an array ***\n";

  $link_arr = vec["$dirname/symlink_link_linkinfo_is_link_link.tmp"];

  /* Testing on soft links */
  echo "\n-- Working with soft links --\n";
  // creating soft link
  var_dump(symlink($filename, $link_arr[0]));
  // check if the link exist
  var_dump(linkinfo($link_arr[0]));
  // check if link is soft link
  var_dump(is_link($link_arr[0]));
  // delete the link created
  unlink($link_arr[0]);
  // clear the cache
  clearstatcache();

  /* Testing on hard links */
  echo "\n-- Working with hard links --\n";
  // creating hard link
  var_dump(link($filename, $link_arr[0]));
  // check if the link exist
  var_dump(linkinfo($link_arr[0]));
  // check if link is soft link; expected: false as this is a hardlink
  var_dump(is_link($link_arr[0]));
  // delete the links created
  unlink($link_arr[0]);
  // clear the cache
  clearstatcache();

  echo "Done\n";

  $dirname = sys_get_temp_dir().'/'.'symlink_link_linkinfo_is_link_variation1';
  unlink("$dirname/symlink_link_linkinfo_is_link_variation1.tmp");
  rmdir("$dirname/test/home");
  rmdir("$dirname/test");
  rmdir($dirname);
}
