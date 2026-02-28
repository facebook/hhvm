<?hh

class AE extends Exception {}
class BE extends Exception {}

function foo () :mixed{
    try {
        try {
            try {
                throw new Exception("try");
            } catch (AE $e) {
                echo "0";
                exit("error");
            } finally {
                echo "1";
            }
        } finally {
            echo "2";
        }
    } catch (BE $e) {
      exit("error");
    } catch (Exception $e) {
        echo "3";
    } finally {
        echo "4";
    }
   return 1;
}
<<__EntryPoint>> function main(): void {
var_dump(foo());
}
