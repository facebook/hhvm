<?hh

require_once('PerfSettings.php');
require_once('Process.php');

final class NginxDaemon extends Process {

  public function __construct(
    private string $tempDir,
    private PerfTarget $target,
  ) {
    parent::__construct('nginx');
  }

  protected function getArguments(): Vector<string> {
    return Vector {
      '-c', $this->getGeneratedConfigFile(),
    };
  }

  protected function getGeneratedConfigFile(): string {
    $path = $this->tempDir.'/nginx.conf';
    if (file_exists($path)) {
      return $path;
    }

    $substitutions = Map {
      '__FASTCGI_PORT__' => PerfSettings::FastCGIPort(),
      '__HTTP_PORT__' => PerfSettings::HttpPort(),
      '__FASTCGI_ADMIN_PORT__' => PerfSettings::FastCGIAdminPort(),
      '__HTTP_ADMIN_PORT__' => PerfSettings::HttpAdminPort(),
      '__NGINX_CONFIG_ROOT__' => __DIR__.'/nginx',
      '__NGINX_TEMP_DIR__' => $this->tempDir,
      '__FRAMEWORK_ROOT__' => $this->target->getSourceRoot(),
    };

    $config = file_get_contents(__DIR__.'/nginx/nginx.conf.in');
    foreach ($substitutions as $find => $replace) {
      $config = str_replace($find, $replace, $config);
    }
    file_put_contents($path, $config);

    return $path;
  }
}
