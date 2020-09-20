<?hh

class :xhp {
  public function __construct($attributes, $children, $file=null, $line=null) {
    $assert = function($cond) {
      if (!$cond) {
        throw new Exception('fail');
      }
    };
    $assert(HH\is_any_array($attributes));
    $assert(HH\is_any_array($children));
    $assert($file === __FILE__);
    $assert($line === $attributes['line']);
  }
}
<<__EntryPoint>> function main(): void {
<xhp line={__LINE__} />;
<xhp line={__LINE__}>text</xhp>;
<xhp line={__LINE__} some-attr="value" />;
<xhp line={__LINE__} attr1="one" a2={2}>text</xhp>;

echo "pass\n";
}
