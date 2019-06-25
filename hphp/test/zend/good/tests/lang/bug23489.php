<?hh
class Test {
  function __construct() {
    ob_start(
      array(
        $this, 'transform'
      )
    );
  }

  function transform($buffer) {
    return 'success';
  }
}
<<__EntryPoint>> function main(): void {
$t = new Test;
echo "failure";
}
