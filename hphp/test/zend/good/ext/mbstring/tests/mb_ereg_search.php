<?hh
<<__EntryPoint>>
function main_entry(): void {
      $str = "\xe4\xb8\xad\xe5\x9b\xbdabc + abc ?!\xef\xbc\x9f\xef\xbc\x81\xe5\xad\x97\xe7\xac\xa6\xef\xbc\x83\xe3\x80\x80china string";

      $reg = "\w+";

      mb_regex_encoding("UTF-8");

      mb_ereg_search_init($str, $reg);
      $r = mb_ereg_search();

      if(!$r)
      {
          echo "null\n";
      }
      else
      {
          $r = mb_ereg_search_getregs(); //get first result
          do
          {
              var_dump($r[0]);
              $r = mb_ereg_search_regs();//get next result
          }
          while($r);
      }
}
