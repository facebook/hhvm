<?hh
require_once __DIR__.'/../Framework.php';

class LessPHP extends Framework {
  <<Override>>
  protected function extraPostComposer(): void {
    // See https://github.com/leafo/lessphp/pull/549
    file_put_contents(
      nullthrows($this->getInstallRoot()).'/phpunit.xml',
      <<<EOF
<phpunit>
  <testsuites>
    <testsuite name="all">
      <directory>tests</directory>
    </testsuite>
  </testsuites>
</phpunit>
EOF
    );
  }
}
