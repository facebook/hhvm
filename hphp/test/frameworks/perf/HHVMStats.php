<?hh

require_once('PerfSettings.php');
require_once('PerfTarget.php');
require_once('PHPEngineStats.php');

trait HHVMStats implements PHPEngineStats {
  abstract protected function getTarget(): PerfTarget;
  abstract protected function adminRequest(string $path): string;

  private function getMetricDefinitions(): Map<string, string> {
    return Map {
      'HHVM CPU instructions' => 'page.inst.all',
      'HHVM CPU usec' => 'page.cpu.all',
      'HHVM wall usec' => 'page.wall.all',
    };
  }

  public function enableStats(): void {
    $endpoints = Vector {
      '/stats-on',
      '/stats-web',
    };

    foreach ($endpoints as $endpoint) {
      $this->adminRequest($endpoint);
    };
  }

  public function collectStats(): Map<string, Map<string, num>> {
    $stats = Map { };

    $metrics = $this->getMetricDefinitions();
    $keys = implode(',', $metrics->values());
    // Given we're only running for one mintue, it should all be in one slot
    // https://github.com/facebook/hhvm/issues/3331
    $json = $this->adminRequest('/stats.json?keys='.$keys);
    $data = json_decode($json, /* assoc array = */ true);

    $combined_hits = 0;
    $combined_stats = Map { };
    foreach ($metrics as $pretty_name => $hhvm_name) {
      $combined_stats[$hhvm_name] = 0;
    }

    foreach ($data['stats']['slot']['pages'] as $page) {
      $url = str_replace(__DIR__, '', $page['url']);
      if ($this->getTarget()->ignorePath($url)) {
        continue;
      }
      $hits = $page['hit'];
      $stats[$url] = $metrics->map(
        $key ==> $page['details'][$key] / $hits
      );

      $stats[$url]['HHVM requests'] = $hits;

      $combined_hits += $hits;
      foreach ($metrics as $key) {
        $combined_stats[$key] += $page['details'][$key];
      }
    }

    $stats['Combined'] = $metrics->map(
      $key ==> $combined_stats[$key] / $combined_hits
    );
    $stats['Combined']['HHVM requests'] = $combined_hits;

    return $stats;
  }
}
