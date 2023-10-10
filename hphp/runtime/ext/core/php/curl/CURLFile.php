<?hh

/**
 * CURLFile can be used to upload a file with CURLOPT_POSTFIELDS.
 */
class CURLFile {
  public string $name = '';
  public string $mime = '';
  public string $postname = '';

  public function __construct(
    string $name,
    string $mime = '',
    string $postname = '',
  ) {
    $this->name = $name;
    $this->mime = $mime;
    $this->postname = $postname;
  }

  public function getFilename(): string {
    return $this->name;
  }

  public function getMimeType(): string {
    return $this->mime;
  }

  public function getPostFilename(): string {
    return $this->postname;
  }

  public function setMimeType(string $mime): void {
    $this->mime = $mime;
  }

  public function setPostFilename(string $postname): void {
    $this->postname = $postname;
  }
}

function curl_file_create(
  string $name,
  string $mime = '',
  string $postname = '',
): CURLFile {
  return new CURLFile($name, $mime, $postname);
}
