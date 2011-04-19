/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <test/test_logger.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/param.h>
#include <runtime/base/util/http_client.h>
#include <runtime/ext/ext_json.h>
#include <runtime/ext/ext_mb.h>
#include <runtime/ext/ext_url.h>
#include <runtime/ext/ext_file.h>

using namespace HPHP;

///////////////////////////////////////////////////////////////////////////////

bool TestLogger::initializeRun() {
  if (!doLog())
    return true;

  char buf[100];
  std::string hostname;
  gethostname(buf, sizeof(buf));
  hostname = buf;

  ArrayInit data(8, false);
  data.set("startedTime",  time(NULL));
  data.set("stillRunning", true);
  data.set("hostname",     hostname);
  data.set("username",     getpwuid(getuid())->pw_name);
  data.set("repository",   getRepoRoot());
  data.set("svnRevision",  getSVNRevision());
  data.set("gitRevision",  getGitRevision());
  data.set("tags",         CREATE_VECTOR2("hphp", "c++"));

  Array dataArr(data.create());

  Array response = postData(CREATE_MAP1("runData", dataArr));

  if (!response["result"]) {
    return false;
  }

  run_id = response["result"]["runId"];

  return true;
}

bool TestLogger::finishRun() {
  if (!doLog())
    return true;
  if (run_id <= 0)
    return false;

  Array data = CREATE_MAP2("runId",   run_id,
                           "runData", CREATE_MAP1("stillRunning", false));

  Array response = postData(data);
  if (response["result"])
    return true;
  return false;
}

bool TestLogger::logTest(Array test) {
  if (!doLog())
    return true;
  if (run_id <= 0)
    return false;

  Array data = CREATE_MAP3("runId",   run_id,
                           "runData", CREATE_MAP1("stillRunning", true),
                           "tests",   CREATE_VECTOR1(test));

  Array response = postData(data);
  if (response["result"])
    return true;
  return false;
}

bool TestLogger::doLog() {
  return log_url != NULL;
}

Array TestLogger::postData(Array arr) {
  HttpClient client;
  StringBuffer response;

  Array data = CREATE_MAP2("method", "recordTestResults",
                           "args", f_json_encode(CREATE_VECTOR1(arr)));

  String str = f_http_build_query(data);

  client.post(log_url, str.c_str(), str.length(), response);

  return f_json_decode(response.detach(), true);
}

std::string TestLogger::getRepoRoot() {
  std::string out;

  if (getOutput("git rev-parse --show-toplevel", out) != -1) {
    // Not all git versions we use know --show-toplevel, if it was just
    // echoed back (this version doesn't know it), fall back to another method
    if (strcmp(out.c_str(), "--show-toplevel") != 0)
      return out;

    // Need to normalize of this commands succeeds
    if (getOutput("git rev-parse --show-cdup", out) != -1)
      return f_realpath(String(out)).toString().data();
  }

  // Fall back to our current directory
  char buf[MAXPATHLEN];
  if (getcwd(buf, sizeof(buf)) == NULL) {
    out.assign(buf);
  } else {
    out.assign("/");
  }
  return out;
}

std::string TestLogger::getGitRevision() {
  std::string commit;

  if (getOutput("git rev-list -1 HEAD", commit) != -1)
    return commit;

  return "";
}

std::string TestLogger::getSVNRevision() {
  std::string out;

  // Redirect stderr to stdout so we don't get annoying messages
  if (getOutput("svn info 2>&1", out) != -1) {
    Variant regs;
    f_mb_ereg("^Revision: (\\d+)$", out, ref(regs));
    return regs[1].getStringData()->data();
  }

  return "";
}

/**
 * Runs a command and gets the first line of output
 */
int TestLogger::getOutput(const char* cmd, std::string &buf) {
  FILE *output = popen(cmd, "r");
  if (output == NULL) {
    buf = "";
    pclose(output);
    return -1;
  }

  char b[1024];
  if (fread(b, sizeof(char), sizeof(b), output) <= 0) {
    buf = "";
    pclose(output);
    return -1;
  }

  buf = b;

  if (pclose(output)) {
    return -1;
  }

  size_t pos = buf.find("\n");
  if (pos < buf.length())
    buf.erase(pos);

  if (buf == "")
    return -1;
  return 1;
}
