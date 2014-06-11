<?hh

class HHVMDocExtension {
  protected bool $verbose = false;
  protected bool $parsed = false;
  protected array $classes = [];
  protected array $functions = [];

  protected function svnScandir(string $path): array {
    $ret = array('dirs' => [], 'files' => []);
    $entries = scandir($path);
    foreach ($entries as $entry) {
      if ($entry[0] == '.') continue;
      if (is_dir("$path/$entry")) {
        $ret['dirs'][$entry] = [
          'name' => $entry,
          'path' => "$path/$entry",
        ];
      } else if (pathinfo($entry, PATHINFO_EXTENSION) === 'xml') {
        $ret['files'][$entry] = [
          'name' => $entry,
          'path' => "$path/$entry",
        ];
      }
    }
    return $ret;
  }

  protected static function GetDescriptionDOM(DOMNode $d): string {
    $ret = '';
    for ($node = $d->firstChild; $node; $node = $node->nextSibling) {
      if ($node->nodeType == XML_TEXT_NODE) {
        $ret .= preg_replace('/\s+/s', ' ', (string)$node->nodeValue);
        continue;
      }
      switch (strtolower($node->nodeName)) {
        case 'function':
          $ret .= self::GetDescriptionDOM($node) . '()';
          break;

        case 'para':
        default:
          $ret .= self::GetDescriptionDOM($node);
      }
    }
    return $ret;
  }

  protected static function GetDescription(SimpleXMLElement $e): string {
    return trim(self::GetDescriptionDOM(dom_import_simplexml($e)));
  }

  protected function getXml(string $path): SimpleXMLElement {
    $xml = file_get_contents($path);
    $xml = preg_replace_callback('@&([a-zA-Z0-9_\.-]*);@s',
    function (array<string> $ents) {
      $ent = $ents[1];
      switch (strtolower($ent)) {
        case 'true':
        case 'false':
        case 'null':
          return strtoupper($ent);
        default:
          return '';
      }
    }, $xml);
    return simplexml_load_string($xml, "SimpleXMLElement",
                                 LIBXML_NOENT|LIBXML_NONET);
  }

  protected function parseFunction(SimpleXMLElement $sxe,
                                   bool $oop): array {
    $f = [];
    // UNSAFE: typechecker doesn't understand __get on $sxe
    foreach ($sxe->refsect1 as $sect) {
      $attr = $sect->attributes();
      if ($attr->role == "description") {
        foreach($sect->methodsynopsis as $synopsis) {
          $fname = (string)$synopsis->methodname;
          $colon = strpos($fname, '::');
          if ($colon === false) {
            if ($oop) { $f['alias'] = $fname; continue; }
            $f['name'] = $fname;
          } else {
            if (!$oop) { $f['alias'] = $fname; continue; }
            $f['class'] = substr($fname, 0, $colon);
            $f['name'] = substr($fname, $colon + 2);
          }
          $f['return']['type'] = (string)$synopsis->type;
          $modifiers = [];
          foreach ($synopsis->modifier as $m) {
            $modifiers[] = (string)$m;
          }
          $f['modifiers'] = $modifiers;
          $f['args'] = [];
          foreach ($synopsis->methodparam as $param) {
            $pn = trim((string)$param->parameter);
            $pa = $param->parameter->attributes();
            $prole = (string)$pa->role;
            if (empty($pn)) continue;
            $arg = [
              'type' => strtolower((string)$param->type),
              'name' => $pn,
              'reference' => ($prole == 'reference'),
            ];
            if (isset($param->initializer)) {
              $arg['default'] = (string)$param->initializer;
            }
            $f['args'][strtolower($pn)] = $arg;
          }
        }
      } elseif ($attr->role == "parameters") {
        foreach ($sect->para->variablelist->varlistentry as $entry) {
          $name = trim((string)$entry->term->parameter);
          if (empty($f['args'][strtolower($name)])) continue;
          $f['args'][strtolower($name)]['desc'] =
                     self::GetDescription($entry->listitem);
        }
      } elseif ($attr->role == "returnvalues") {
        $f['return']['desc'] = self::GetDescription($sect);
      }
    }
    if (!empty($f['name'])) {
      $f['desc'] = (string)$sxe->refnamediv->refpurpose;
      if (empty($f['class'])) {
        $this->functions[strtolower($f['name'])] = $f;
      } else {
        $this->classes[strtolower($f['class'])]
                      ['functions'][strtolower($f['name'])] = $f;
      }
    }
  }

  protected function parseFunctions(string $path): void {
    $dirinfo = $this->svnScandir($path);
    foreach($dirinfo['files'] as $file) {
      $xml = $this->getXml($file['path']);
      $this->parseFunction($xml, true);
      $this->parseFunction($xml, false);
    }
  }

  protected function parseClass(string $path): void {
    $sxe = $this->getXml($path);
    // UNSAFE: typechecker doesn't understand __get on $sxe
    foreach ($sxe->partintro->section as $sect) {
      $xmlattrs = $sect->attributes('xml', true);
      if (substr($xmlattrs->id, -6) == '.intro') {
        $name = substr($xmlattrs->id, 0, -6);
        $this->classes[strtolower($name)]['intro'] =
               self::GetDescription($sect);
      } elseif (substr($xmlattrs->id, -9) == '.synopsis' &&
                !empty($sect->classsynopsis)) {
        $name = substr($xmlattrs->id, 0, -9);
        $synopsis = $sect->classsynopsis;
        $this->classes[strtolower($name)]['name'] = ucfirst($name);
        foreach($synopsis->ooclass as $ooclass) {
          $modifier = (string)$ooclass->modifier;
          $cn = (string)$ooclass->classname;
          if (empty($modifier)) {
            $this->classes[strtolower($name)]['name'] = $cn;
          } elseif ($modifier == 'extends') {
            $this->classes[strtolower($name)]['extends'] = $cn;
          } elseif ($modifier == 'implements') {
            $this->classes[strtolower($name)]['implements'][] = $cn;
          }
        }
      } elseif (substr($xmlattrs->id, -6) == '.props' &&
                !empty($sect->variablelist)) {
        $name = substr($xmlattrs->id, 0, -6);
        foreach ($sect->variablelist->varlistentry as $field) {
          $propname = (string)$field->term->varname;
          $this->classes[strtolower($name)]['props'][strtolower($propname)] = [
            'name' => $propname,
            'desc' => self::GetDescription($field->listitem),
          ];
        }
      }
    }
  }

  protected function fixupClasses(): void {
    foreach ($this->classes as &$class) {
      if (!empty($class['functions']) &&
           empty($class['name'])) {
        foreach($class['functions'] as $func) {
          $class['name'] = $func['class'];
          break;
        }
      }
    }
  }

  protected function parse(): this {
    if ($this->parsed) return $this;

    $url = $this->root.
           '/en/reference/'.
           urlencode($this->name);
    $dirinfo = $this->svnScandir($url);
    foreach($dirinfo['files'] as $file) {
      $this->parseClass($file['path']);
    }

    foreach ($dirinfo['dirs'] as $dir) {
      $this->parseFunctions($dir['path']);
    }
    $this->fixupClasses();

    $this->parsed = true;
    return $this;
  }

  public function __construct(protected string $name,
                              protected string $root):
                  void {
    if (!file_exists($this->root.'/en/reference') ||
        !is_dir($this->root.'/en/reference')) {
      throw new Exception("{$this->root} does not appear to be ".
                          "a phpdoc checkout.  See http://php.net/svn ".
                          "for instructions on how to checkout phpdoc-en.");
    }
  }

  public function getFunctions(): array { return $this->parse()->functions; }
  public function getClasses(): array { return $this->parse()->classes; }
  public function setVerbose(bool $verbose): this {
    $this->verbose = $verbose;
    return $this;
  }
}

