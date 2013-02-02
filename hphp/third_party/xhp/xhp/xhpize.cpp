/*
  +----------------------------------------------------------------------+
  | XHP                                                                  |
  +----------------------------------------------------------------------+
  | Copyright (c) 2009 - 2010 Facebook, Inc. (http://www.facebook.com)          |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE.PHP, and is    |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
*/

#include "xhp_preprocess.hpp"
#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[]) {
  bool in_place = false, dry_run = false;
  vector<string> files;

  // Parse args
  for (int ii = 1; ii < argc; ++ii) {
    if (strcmp(argv[ii], "-i") == 0) {
      in_place = true;
    } else if (strcmp(argv[ii], "-d") == 0) {
      dry_run = true;
    } else if (strcmp(argv[ii], "-h") == 0 || strcmp(argv[ii], "-?") == 0) {
      cerr<< argv[0] << " -i [files] | " << argv[0] << " [file]\n";
      return 1;
    } else {
      files.push_back(argv[ii]);
    }
  }

  // Sanity checking
  if (in_place && files.size() == 0) {
    cerr<< "In place mode must be used with at least one file.\n";
    return 1;
  } else if (!in_place && files.size() > 1) {
    cerr<< "Multiple files must be used with in place mode.\n";
    return 1;
  } else if (files.size() == 0) {
    files.push_back("-");
  }

  // Parse
  for (vector<string>::iterator ii = files.begin(); ii != files.end(); ++ii) {

    ifstream inputFile;
    istream *inputStream;
    if (*ii == "-") {
      inputStream = &cin;
    } else {
      inputFile.open(ii->c_str());
      inputStream = &inputFile;
    }

    string code, error;
    uint32_t errLine;
    XHPResult result = xhp_preprocess(*inputStream, code, false, error, errLine);
    inputFile.close();
    if (result == XHPRewrote) {
      if (in_place) {
        if (!dry_run) {
          ofstream outputFile(ii->c_str());
          outputFile<< code;
          outputFile.close();
        }
      } else {
        cout<< code;
        cout.flush();
      }
      cerr<< "File `"<<(*ii)<<"` xhpized.\n";
    } else if (result == XHPErred) {
      cerr<< "Error parsing file `"<<(*ii)<<"`!!\n" << error << " on line " <<
        errLine << endl;;
    }
  }

  return 0;
}
