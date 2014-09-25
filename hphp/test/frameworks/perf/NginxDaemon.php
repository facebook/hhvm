<?hh

require_once('PerfOptions.php');
require_once('PerfSettings.php');
require_once('Process.php');

final class NginxDaemon extends Process {

  public function __construct(
    private PerfOptions $options,
    private PerfTarget $target,
  ) {
    parent::__construct($this->options->nginx);
  }

  public function start(): void {
    parent::startWorker(
      $this->options->daemonOutputFileName('nginx'),
      $this->options->delayProcessLaunch,
      $this->options->traceSubProcess,
    );
  }

  public function clearAccessLog(): void {
    $log = $this->options->tempDir.'/access.log';
    invariant(
      file_exists($log),
      'access log does not exist, but attempted to clear it'
    );
    unlink($log);
    $pid = $this->getPid();
    if ($pid !== null) {
      posix_kill($pid, SIGUSR1);
    }
  }

  public function collectStats(): Map<string, Map<string, num>> {
    $combined_codes = [];
    $combined_hits = 0;
    $combined_time = 0;
    $combined_bytes = 0;

    $page_results = Map { };

    // Custom format: '$status $body_bytes_sent $request_time "$request"'
    $log = file_get_contents($this->options->tempDir.'/access.log');
    $entries = explode("\n", trim($log));
    $entries_by_request = array();
    foreach ($entries as $entry) {
      $request = explode('"', $entry)[1];
      $entries_by_request[$request][] = $entry;
    }

    foreach ($entries_by_request as $request => $entries) {
      $request_hits = count($entries);
      $combined_hits += $request_hits;
      $page_results[$request] = Map {
        'Nginx hits' => $request_hits,
        'Nginx avg bytes' => 0,
        'Nginx avg time' => 0,
      };

      foreach ($entries as $entry) {
        $parts = explode(' ', $entry);
        list($code, $bytes, $time) = [$parts[0], $parts[1], $parts[2]];

        $combined_codes[$code]++;
        $combined_bytes += $bytes;
        $combined_time += $time;

        $page_results[$request]['Nginx avg bytes'] += $bytes;
        $page_results[$request]['Nginx avg time'] += $time;
        $code_key = 'Nginx '.$code;
        if ($page_results[$request]->containsKey($code_key)) {
          $page_results[$request][$code_key]++;
        } else {
          $page_results[$request][$code_key] = 1;
        }
      }
      $page_results[$request]['Nginx avg bytes'] /= (float) $request_hits;
      $page_results[$request]['Nginx avg time'] /= (float) $request_hits;
    }
    $page_results['Combined'] = Map {
      'Nginx hits' => $combined_hits,
      'Nginx avg bytes' => ((float) $combined_bytes) / $combined_hits,
      'Nginx avg time' => ((float) $combined_time) / $combined_hits,
    };
    foreach ($combined_codes as $code => $count) {
      $page_results['Combined']['Nginx '.$code] = $count;
    }
    return $page_results;
  }

  protected function getArguments(): Vector<string> {
    return Vector {
      '-c', $this->getGeneratedConfigFile(),
    };
  }

  protected function getGeneratedConfigFile(): string {
    $path = $this->options->tempDir.'/nginx.conf';
    if (file_exists($path)) {
      return $path;
    }

    $substitutions = Map {
      '__FASTCGI_PORT__' => PerfSettings::FastCGIPort(),
      '__HTTP_PORT__' => PerfSettings::HttpPort(),
      '__FASTCGI_ADMIN_PORT__' => PerfSettings::FastCGIAdminPort(),
      '__HTTP_ADMIN_PORT__' => PerfSettings::HttpAdminPort(),
      '__NGINX_CONFIG_ROOT__' => __DIR__.'/nginx',
      '__NGINX_TEMP_DIR__' => $this->options->tempDir,
      '__NGINX_KEEPALIVE_TIMEOUT__' =>
        (int)$this->options->maxdelayNginxKeepAlive,
      '__NGINX_FASTCGI_READ_TIMEOUT__' =>
        (int)$this->options->maxdelayNginxFastCGI,
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
