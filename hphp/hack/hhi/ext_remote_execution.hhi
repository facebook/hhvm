<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

namespace HH\RemoteExecution {

/**
 * Uploads a file using CAS.
 *
 * @param string $filepath  - Path to the file to upload.
 * @param string $usecase  - CAS use case.
 *
 * @return string  - CAS digest of the uploaded file.
 */
function gen_upload_file(string $filepath, string $usecase)[]: Awaitable<string>;

/**
 * Downloads a file using CAS.
 *
 * @param string $casdigest - CAS digest of the file to download.
 * @param string $dest  - Destination file path.
 * @param string $usecase  - CAS use case.
 *
 * @return bool - Whether the download was successful.
 */
function gen_download_file(string $casdigest, string $destpath, string $usecase)[]: Awaitable<bool>;

} // namespace HH\Facts
