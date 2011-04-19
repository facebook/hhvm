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

#include <runtime/base/shared/shared_store_stats.h>
#include <runtime/base/runtime_option.h>

#include <util/json.h>
#include <pcre.h>

using namespace std;

namespace HPHP {
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Helpers to write JSON entry

static void writeEntryInt(ostream& out, const char *name, int64 value,
                          int indent, bool last = false) {
  for (int i = 0; i < indent ; i++) {
    out << "  ";
  }
  if (last) {
    out << "\"" << JSON::Escape(name) << "\" : " << value << "\n";
  } else {
    out << "\"" << JSON::Escape(name) << "\" : " << value << ",\n";
  }
}

//////////////////////////////////////////////////////////////////////////////
// Helpers to aggregate keys

static bool regex_match(const pcre *pattern, const char *subject) {
  int ovector[3];
  return pcre_exec(pattern, NULL, subject, strlen(subject), 0, 0, ovector,
                   3) >= 0;
}

static bool regex_replace(const pcre *pattern, const char *subject,
                         const char *replacement, char *result,
                         size_t resultLen) {
  int ovector[3];
  int match = 0;
  int last = 0;
  int replacementlen = strlen(replacement);
  int offset = 0;
  int remain = resultLen;
  while (true) {
    int cnt = pcre_exec(pattern, NULL, subject, strlen(subject), last, 0,
                        ovector, 3);
    if (cnt < 0) {
      // error or no match
      break;
    }

    match++;
    // copy the string before match
    int prelen = ovector[0] - last;
    int copylen = prelen > remain ? remain : prelen;
    const char *start = subject + last;
    memcpy(result + offset, start, copylen);
    offset += copylen;
    remain -= copylen;
    if (remain == 0) break;

    // replace the match part
    copylen = replacementlen > remain ? remain : replacementlen;
    memcpy(result + offset, replacement, copylen);
    offset += copylen;
    remain -= copylen;
    if (remain == 0) break;

    last = ovector[1];
  }
  if (match > 0 && remain > 0) {
    // copy the last piece
    const char *start = subject + last;
    strncpy(result + offset, start, remain);
  }
  return match > 0;
}

static void normalizeKey(const char *key, char *normalizedKey, size_t outlen) {
  // key is a NULL-terminated string, normalize it and store it in
  // normalizedKey, with no longer than outlen, may not null terminated
  // if outlen is too small

  vector<std::string> &specialPrefix = RuntimeOption::APCSizeSpecialPrefix;
  vector<std::string> &prefixReplace = RuntimeOption::APCSizePrefixReplace;
  for (unsigned int i = 0; i < specialPrefix.size(); i++) {
    const char *prefix = specialPrefix[i].c_str();
    if (strncmp(key, prefix, specialPrefix[i].length()) == 0) {
      strncpy(normalizedKey, prefixReplace[i].c_str(), outlen);
      return;
    }
  }
  vector<std::string> &specialMiddle = RuntimeOption::APCSizeSpecialMiddle;
  vector<std::string> &middleReplace = RuntimeOption::APCSizeMiddleReplace;
  for (unsigned int i = 0; i < specialMiddle.size(); i++) {
    const char *middle = specialMiddle[i].c_str();
    if (strstr(key, middle) != NULL) {
      strncpy(normalizedKey, middleReplace[i].c_str(), outlen);
      return;
    }
  }

  const char *error;
  int erroffset;
  static const pcre *re_lower =
    pcre_compile("/[a-z]/", 0, &error, &erroffset, NULL);

  if (index(key, ':') == NULL && !regex_match(re_lower, key)) {
    strncpy(normalizedKey, "ALL_CAPS_N_NUMBERS", outlen);
    return;
  }

  static const pcre *re_hash =
    pcre_compile("[a-f0-9]{8,}", 0, &error, &erroffset, NULL);
  static const char *re_hash_replace = "{H}";
  static const pcre *re_number =
    pcre_compile("-?\\d+", 0, &error, &erroffset, NULL);
  static const char *re_number_replace = "{N}";
  static const pcre *re_locale =
    pcre_compile("\\b[a-z][a-z]_[A-Z][A-Z]\\b", 0, &error, &erroffset, NULL);
  static const char *re_locale_replace = "{L}";
  static const pcre *re_i18n =
    pcre_compile("^i{N}n", 0, &error, &erroffset, NULL);
  static const char *re_i18n_replace = "i18n";

  char *tempBuf = (char *)calloc(outlen + 1, 1);
  strncpy(tempBuf, key, outlen);
  tempBuf[outlen] = '\0';
  bool isReplaced;

  isReplaced = regex_replace(re_locale, tempBuf, re_locale_replace,
                             normalizedKey, outlen);
  if (isReplaced) {
    strncpy(tempBuf, normalizedKey, outlen);
    tempBuf[outlen] = '\0';
  }
  isReplaced = regex_replace(re_hash, tempBuf, re_hash_replace,
                             normalizedKey, outlen);
  if (isReplaced) {
    strncpy(tempBuf, normalizedKey, outlen);
    tempBuf[outlen] = '\0';
  }
  isReplaced = regex_replace(re_number, tempBuf, re_number_replace,
                             normalizedKey, outlen);
  if (isReplaced) {
    strncpy(tempBuf, normalizedKey, outlen);
    tempBuf[outlen] = '\0';
  }
  isReplaced = regex_replace(re_i18n, tempBuf, re_i18n_replace,
                             normalizedKey, outlen);
  if (isReplaced) {
    strncpy(tempBuf, normalizedKey, outlen);
    tempBuf[outlen] = '\0';
  }
  strncpy(normalizedKey, tempBuf, outlen);
  free(tempBuf);
}

//////////////////////////////////////////////////////////////////////////////
// Helpers to handle profile entry

void SharedValueProfile::calcInd(StringData *key, SharedVariant *variant) {
  keySize = key->size();
  variant->getStats(&var);
  totalSize = keySize + var.dataTotalSize;
}

void SharedValueProfile::addToGroup(SharedValueProfile *ind) {
  ASSERT(isGroup);
  totalSize += ind->totalSize;
  keySize += ind->keySize;
  var.addChildStats(&ind->var);
  if (keyCount >= 0) {
    // When not counting prime, keyCount may get below 0 if primed
    // keys get deleted. In this case, there will be inaccuracy but
    // it is usually small. This check is to prevent divide by zero error
    ttl = (ttl * keyCount + ind->ttl) / (keyCount + 1);
  }
  keyCount++;
}

void SharedValueProfile::removeFromGroup(SharedValueProfile *ind) {
  ASSERT(isGroup);
  totalSize -= ind->totalSize;
  keySize -= ind->keySize;
  var.removeChildStats(&ind->var);
  keyCount--;
}

//////////////////////////////////////////////////////////////////////////////
// Definition of static members
#define MAX_KEY_LEN  120

int32 SharedStoreStats::s_keyCount = 0;
int32 SharedStoreStats::s_keySize = 0;
int32 SharedStoreStats::s_variantCount = 0;
int64 SharedStoreStats::s_dataSize = 0;
int64 SharedStoreStats::s_dataTotalSize = 0;
int64 SharedStoreStats::s_deleteSize = 0;
int64 SharedStoreStats::s_replaceSize = 0;

Mutex SharedStoreStats::s_lock;
SharedStoreStats::StatsMap SharedStoreStats::s_statsMap,
                           SharedStoreStats::s_detailMap;

//////////////////////////////////////////////////////////////////////////////
// Helpers for reporting and global aggregation

string SharedStoreStats::report_basic() {
  ostringstream out;
  out << "{\n";
  out << "\"APCSizeStats\": {\n";
  writeEntryInt(out, "Key_Count", s_keyCount, 1);
  writeEntryInt(out, "Size_Total", s_keySize + s_dataTotalSize, 1);
  writeEntryInt(out, "Size_Key", s_keySize, 1);
  writeEntryInt(out, "Size_Data", s_dataTotalSize, 1, true);
  out << "}\n";
  out << "}\n";
  return out.str();
}

string SharedStoreStats::report_basic_flat() {
  ostringstream out;
  out << "{ " << "\"hphp.apc.size_total\":" << s_keySize + s_dataTotalSize
      << ", " << "\"hphp.apc.key_count\":" << s_keyCount
      << ", " << "\"hphp.apc.size_key\":" << s_keySize
      << ", " << "\"hphp.apc.size_data\":" << s_dataTotalSize
      << "}\n";
  return out.str();
}

string SharedStoreStats::report_keys() {
  ostringstream out;
  out << "{\n";
  out << "\"APCSizeKeyStats\": {\n";
  StatsMap::iterator iter;
  bool first = true;
  for (iter = s_statsMap.begin(); iter != s_statsMap.end(); ++iter) {
    ASSERT(iter->second->isGroup);
    if (first) first = false;
    else out << ",\n";
    out << "  \"" << iter->first << "\": {\n";
    writeEntryInt(out, "TotalSize", iter->second->totalSize, 2);
    writeEntryInt(out, "Count", iter->second->keyCount, 2);
    writeEntryInt(out, "AvgTTL", iter->second->ttl, 2);
    writeEntryInt(out, "KeySize", iter->second->keySize, 2);
    writeEntryInt(out, "DataSize", iter->second->var.dataTotalSize, 2, true);
    out << "  }";
  }
  out << "\n}\n";
  out << "}\n";
  return out.str();
}

bool SharedStoreStats::snapshot(const char *filename, std::string& keySample) {
  ofstream out(filename);
  if (out.fail()) {
    return false;
  }
  char nkeySample[MAX_KEY_LEN + 1];
  if (keySample != "") {
    normalizeKey(keySample.c_str(), nkeySample, MAX_KEY_LEN);
    nkeySample[MAX_KEY_LEN] = '\0';
  }
  lock();
  out << "{\n";
  out << "\"APCSizeDetail\": {\n";
  StatsMap::iterator iter;
  bool first = true;
  time_t now = time(NULL);
  for (iter = s_detailMap.begin(); iter != s_detailMap.end();  ++iter) {
    ASSERT(!iter->second->isGroup);
    if (keySample != "") {
      char nkey[MAX_KEY_LEN + 1];
      normalizeKey(iter->first, nkey, MAX_KEY_LEN);
      nkey[MAX_KEY_LEN] = '\0';
      if (strcmp(nkey, nkeySample) != 0) continue;
    }
    if (!iter->second->isValid) continue;
    if (first) first = false;
    else out << ",\n";
    out << "  \"" << iter->first << "\": {\n";
    writeEntryInt(out, "TotalSize", iter->second->totalSize, 2);
    writeEntryInt(out, "Prime", (int64)iter->second->isPrime, 2);
    writeEntryInt(out, "TTL", iter->second->ttl, 2);
    writeEntryInt(out, "KeySize", iter->second->keySize, 2);
    writeEntryInt(out, "DataSize", iter->second->var.dataTotalSize, 2);
    writeEntryInt(out, "StoreCount", iter->second->storeCount, 2);
    writeEntryInt(out, "SinceLastStore", now - iter->second->lastStoreTime, 2);
    writeEntryInt(out, "DeleteCount", iter->second->deleteCount, 2);
    if (iter->second->deleteCount > 0) {
      writeEntryInt(out, "SinceLastDelete", now - iter->second->lastDeleteTime,
                    2);
    }
    if (RuntimeOption::EnableAPCFetchStats) {
      writeEntryInt(out, "FetchCount", iter->second->fetchCount, 2);
      if (iter->second->fetchCount > 0) {
        writeEntryInt(out, "SinceLastFetch", now - iter->second->lastFetchTime,
                      2);
      }
    }
    writeEntryInt(out, "VariantCount", iter->second->var.variantCount, 2);
    writeEntryInt(out, "UserDataSize", iter->second->var.dataSize, 2, true);
    out << "  }";
  }
  out << "\n}\n";
  out << "}\n";
  unlock();
  out.close();
  return true;
}

void SharedStoreStats::remove(SharedValueProfile *svp, bool replace) {
  s_dataSize -= svp->var.dataSize;
  s_dataTotalSize -= svp->var.dataTotalSize;
  s_variantCount -= svp->var.variantCount;
  s_keySize -= svp->keySize;
  if (replace) s_replaceSize += svp->totalSize;
  else s_deleteSize += svp->totalSize;
  s_keyCount--;
}

void SharedStoreStats::add(SharedValueProfile *svp) {
  s_dataSize += svp->var.dataSize;
  s_dataTotalSize += svp->var.dataTotalSize;
  s_variantCount += svp->var.variantCount;
  s_keySize += svp->keySize;
  s_keyCount++;
}

//////////////////////////////////////////////////////////////////////////////
// Hooks

void SharedStoreStats::addDirect(int32 keySize, int32 dataTotal) {
  lock();
  s_keyCount++;
  s_keySize += keySize;
  s_dataTotalSize += dataTotal;
  unlock();
}

void SharedStoreStats::removeDirect(int32 keySize, int32 dataTotal) {
  lock();
  s_keyCount--;
  s_keySize -= keySize;
  s_dataTotalSize -= dataTotal;
  unlock();
}

void SharedStoreStats::updateDirect(int32 dataTotalOld, int32 dataTotalNew) {
  lock();
  s_dataTotalSize -= dataTotalOld;
  s_dataTotalSize += dataTotalNew;
  unlock();
}

void SharedStoreStats::onClear() {
  lock();
  StatsMap::iterator iter;
  for (iter = s_statsMap.begin(); iter != s_statsMap.end();) {
    delete (iter++)->second;
  }
  s_statsMap.clear();
  if (RuntimeOption::EnableAPCSizeDetail) {
    for (iter = s_detailMap.begin(); iter != s_detailMap.end();) {
      delete (iter++)->second;
    }
    s_detailMap.clear();
  }
  resetStats();
  unlock();
}

void SharedStoreStats::onDelete(StringData *key, SharedVariant *var,
                                bool replace) {
  char normalizedKey[MAX_KEY_LEN + 1];

  if (RuntimeOption::EnableAPCSizeGroup) {
    normalizeKey(key->data(), normalizedKey, MAX_KEY_LEN);
    normalizedKey[MAX_KEY_LEN] = '\0';
  }

  SharedValueProfile svpTemp;
  // Calculate size of the variant
  svpTemp.calcInd(key, var);

  lock();

  if (RuntimeOption::EnableAPCSizeDetail) {
    SharedValueProfile *svp;
    StatsMap::iterator iter = s_detailMap.find((char*)key->data());
    if (iter != s_detailMap.end()) {
      svp = iter->second;
      ASSERT(svp->isValid);
      if (!replace) {
        svp->isValid = false;
        svp->deleteCount++;
        svp->lastDeleteTime = time(NULL);
      }
    } else {
      ASSERT(false);
    }
  }

  if (RuntimeOption::EnableAPCSizeGroup) {
    StatsMap::iterator iter = s_statsMap.find(normalizedKey);
    if (iter != s_statsMap.end()) {
      SharedValueProfile *group = iter->second;
      group->removeFromGroup(&svpTemp);
    } else {
      ASSERT(false);
    }
  }
  unlock();
}

void SharedStoreStats::onGet(StringData *key, SharedVariant *var) {
  lock();
  StatsMap::iterator iter = s_detailMap.find((char*)key->data());
  if (iter != s_detailMap.end()) {
    SharedValueProfile *svpInd = iter->second;
    svpInd->lastFetchTime = time(NULL);
    svpInd->fetchCount++;
  } else {
    ASSERT(false);
  }
  unlock();
}

void SharedStoreStats::onStore(StringData *key, SharedVariant *var,
                               int64 ttl, bool prime) {
  char normalizedKey[MAX_KEY_LEN + 1];

  SharedValueProfile *svpInd;

  svpInd = new SharedValueProfile(key->data());
  svpInd->calcInd(key, var);
  svpInd->ttl = ttl > 0 && ttl < 48*3600 ? ttl : 48*3600;

  if (RuntimeOption::EnableAPCSizeGroup) {
    // Here so that it is out of critical section
    normalizeKey(key->data(), normalizedKey, MAX_KEY_LEN);
    normalizedKey[MAX_KEY_LEN] = '\0';
  }

  lock();

  if (RuntimeOption::EnableAPCSizeGroup) {
    SharedValueProfile *group;
    StatsMap::iterator iter = s_statsMap.find(normalizedKey);
    if (iter == s_statsMap.end()) {
      group = new SharedValueProfile(normalizedKey);
      group->isGroup = true;
      group->keyCount = 0;
      s_statsMap[group->key] = group;
    } else {
      group = iter->second;
    }
    group->addToGroup(svpInd);
  }

  if (RuntimeOption::EnableAPCSizeDetail) {
    StatsMap::iterator iter = s_detailMap.find(svpInd->key);
    if (iter == s_detailMap.end()) {
      s_detailMap[svpInd->key] = svpInd;
    } else {
      SharedValueProfile *existing = iter->second;
      // update size but keep other stats
      existing->totalSize = svpInd->totalSize;
      existing->keySize = svpInd->keySize;
      existing->var = svpInd->var;
      delete svpInd;
      svpInd = existing;
    }
    svpInd->isValid = true;
    svpInd->storeCount++;
    svpInd->lastStoreTime = time(NULL);
    if (prime) svpInd->isPrime = true;
  } else {
    delete svpInd;
  }

  unlock();
}

///////////////////////////////////////////////////////////////////////////////
}
