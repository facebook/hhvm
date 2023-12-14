<?hh

class AE extends Exception {}
class BE extends Exception {}

function foo () :mixed{
    try {
        try {
            try {
                try {
                   echo "1";
                   throw new Exception("try");
                } catch (AE $e) {
                   exit("error");
                } finally {
                   echo "2";
                }
            } finally {
                echo "3";
            }
        } catch (BE $e) {
            exit("error");
        } finally {
            echo "4";
        }
    } catch (Exception $e) {
        echo "5";
    } catch (AE $e) {
        exit("error");
    } finally {
        echo "6";
    }
   return 7;
}
<<__EntryPoint>> function main(): void {
var_dump(foo());
}
