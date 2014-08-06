<?hh
require_once __DIR__.'/../Framework.php';

class Predis extends Framework {
  protected function install(): void {
    parent::install();
    verbose("Creating a phpunit.xml for running the Predis tests.\n");
    $phpunit_xml = <<<XML
<?xml version="1.0" encoding="UTF-8"?>
<phpunit bootstrap="tests/bootstrap.php" colors="true">
    <testsuites>
        <testsuite name="Predis Test Suite">
            <directory>tests/Predis/</directory>
        </testsuite>
    </testsuites>
    <groups>
        <exclude>
            <group>ext-phpiredis</group>
            <group>ext-curl</group>
            <group>realm-webdis</group>
            <group>connected</group>
        </exclude>
    </groups>
    <filter>
        <whitelist>
            <directory suffix=".php">src/</directory>
        </whitelist>
    </filter>
    <php>
        <const name="REDIS_SERVER_VERSION" value="2.8" />
        <const name="REDIS_SERVER_HOST" value="127.0.0.1" />
        <const name="REDIS_SERVER_PORT" value="6379" />
        <const name="REDIS_SERVER_DBNUM" value="15" />
        <const name="WEBDIS_SERVER_HOST" value="127.0.0.1" />
        <const name="WEBDIS_SERVER_PORT" value="7379" />
    </php>
</phpunit>
XML;
    file_put_contents($this->getTestPath()."/phpunit.xml.dist", $phpunit_xml);
  }
}
