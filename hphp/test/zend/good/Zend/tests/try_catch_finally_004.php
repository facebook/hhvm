<?hh
function dummy($msg) :mixed{
   var_dump($msg);
}
<<__EntryPoint>> function main(): void {
try {
    try {
        var_dump("try");
        return;
    } catch (Exception $e) {
        dummy("catch");
        throw $e;
    } finally {
        dummy("finally");
    }
} catch (Exception $e) {
  dummy("catch2");
} finally {
  dummy("finally2");
}
var_dump("end");
}
