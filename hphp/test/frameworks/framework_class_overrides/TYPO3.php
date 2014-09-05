<?hh
require_once __DIR__.'/../Framework.php';

class Typo3 extends Framework {
  protected $additionalFolders = array('typo3temp', 'uploads',
                                       'typo3conf', 'typo3conf/ext');

  public function __construct(string $name) {
    $typo3path = Options::$frameworks_root . '/typo3';
    $env_vars = Map { "TYPO3_PATH_WEB" =>  $typo3path };
    $tc = get_runtime_build().' '.__DIR__.
      '/../framework_downloads/typo3/bin/phpunit';
    parent::__construct($name, $tc, $env_vars, null,
                        false, TestFindModes::TOKEN);
  }

  protected function install(): void {
    parent::install();
    // comment out 2 methods from interfaces which couses troubles
    // because of https://github.com/facebook/hhvm/issues/2527
    $filePath = $this->getInstallRoot() .
                '/typo3/sysext/core/Resources/PHP/TYPO3.Flow/Classes/'.
                'TYPO3/Flow/Package/PackageManagerInterface.php';
    file_put_contents($filePath,
                      str_replace('public function initialize(',
                                  '//public function initialize(',
                                  file_get_contents($filePath)));
    $filePath = $this->getInstallRoot() .
                '/typo3/sysext/core/Resources/PHP/TYPO3.Flow/Classes/'.
                'TYPO3/Flow/Package/PackageInterface.php';
    file_put_contents($filePath,
                      str_replace('public function boot(',
                                  '//public function boot(',
                                  file_get_contents($filePath)));

    verbose("Creating a phpunit xml configuration file for running the TYPO3 tests.\n");
	$phpunit_xml = <<<XML
<?xml version="1.0"?>
<phpunit backupGlobals="true" backupStaticAttributes="false" bootstrap="UnitTestsBootstrap.php" colors="false" convertErrorsToExceptions="true" convertWarningsToExceptions="true" forceCoversAnnotation="false" processIsolation="false" stopOnError="false" stopOnFailure="false" stopOnIncomplete="false" stopOnSkipped="false" strict="false" verbose="false">
	<testsuites>
		<testsuite name="Core tests">
			<directory>../../../../typo3/sysext/core/Tests/Unit/</directory>
		</testsuite>
		<testsuite name="Core modules">
			<directory>../../../../typo3/sysext/backend/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/belog/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/beuser/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/dbal/Tests/Unit/</directory>

			<directory>../../../../typo3/sysext/documentation/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/extbase/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/extensionmanager/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/felogin/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/fluid/Tests/Unit/</directory>

			<!--For some reason hhvm fatals with this folder, but runs OK if we list all test files separately-->
			<!--<directory>../../../../typo3/sysext/form/Tests/Unit/</directory>-->
			<directory>../../../../typo3/sysext/frontend/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/impexp/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/indexed_search/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/install/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/lang/Tests/Unit/</directory>

			<directory>../../../../typo3/sysext/lowlevel/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/recordlist/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/rsauth/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/saltedpassword/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/scheduler/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/sv/Tests/Unit/</directory>
			<directory>../../../../typo3/sysext/workspaces/Tests/Unit/</directory>
		</testsuite>
		<testsuite name="Core legacy tests">
			<directory>../../../../typo3/sysext/core/Tests/Legacy/</directory>
		</testsuite>
		<testsuite name="Suite integrity tests">
			<directory>../../../../typo3/sysext/core/Tests/Integrity/</directory>
		</testsuite>
		<testsuite name="Form extension">
			<file>../../../../typo3/sysext/form/Tests/Unit/Domain/Factory/TypoScriptFactoryTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/AlphanumericFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/StripNewLinesFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/RegExpFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/DigitFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/TrimFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/TitleCaseFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/AlphabeticFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/CurrencyFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/UpperCaseFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/LowerCaseFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/IntegerFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Filter/RemoveXssFilterTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/PostProcess/MailPostProcessorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/PostProcess/PostProcessorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/InArrayValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/AlphabeticValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/UriValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/IntegerValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/FloatValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/DateValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/EmailValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/RequiredValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/AlphanumericValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/IpValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/FileMinimumSizeValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/DigitValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/FileAllowedTypesValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/EqualsValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/FileMaximumSizeValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/LengthValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/BetweenValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/GreaterThanValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/RegExpValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/Validation/LessThanValidatorTest.php</file>
			<file>../../../../typo3/sysext/form/Tests/Unit/View/Mail/Html/Element/AbstractElementViewTest.php</file>
		</testsuite>
	</testsuites>
</phpunit>
XML;

    file_put_contents($this->getTestPath()."/HHVMUnitTests.xml", $phpunit_xml);


  }

  protected function isInstalled(): bool {
    foreach($this->additionalFolders as $folder) {
      if(!file_exists($this->getInstallRoot() . '/' . $folder)) {
        if(file_exists($this->getInstallRoot())) {
          remove_dir_recursive($this->getInstallRoot());
        }
        return false;
      }
    }
    return parent::isInstalled();
  }
}
