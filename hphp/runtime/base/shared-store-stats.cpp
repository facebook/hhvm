/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/shared-store-stats.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/util/json.h"
#include <pcre.h>

using std::ostream;
using std::ostringstream;

namespace HPHP {
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Helpers to write JSON entry

static void writeEntryInt(ostream& out, const char *name, int64_t value,
                          bool last = false, int indent = 0,
                          bool newline = false) {
  for (int i = 0; i < indent ; i++) {
    out << "  ";
  }
  if (last) {
    out << "\"" << JSON::Escape(name) << "\":" << value;
  } else {
    out << "\"" << JSON::Escape(name) << "\":" << value << ", ";
  }
  if (newline) {
    out << "\n";
  }
}

static void writeEntryStr(ostream& out, const char *name, const char* value,
                          bool last = false, int indent = 0,
                          bool newline = false) {
  for (int i = 0; i < indent ; i++) {
    out << "  ";
  }
  if (last) {
    out << "\"" << JSON::Escape(name) << "\":\"" << value << "\"";
  } else {
    out << "\"" << JSON::Escape(name) << "\":\"" << value << "\", ";
  }
  if (newline) {
    out << "\n";
  }
}

//////////////////////////////////////////////////////////////////////////////
// Helpers to aggregate keys

static bool regex_match(const pcre *pattern, const char *subject) {
  int ovector[3];
  return pcre_exec(pattern, nullptr, subject, strlen(subject), 0, 0, ovector,
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
    int cnt = pcre_exec(pattern, nullptr, subject, strlen(subject), last, 0,
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
    if (strstr(key, middle) != nullptr) {
      strncpy(normalizedKey, middleReplace[i].c_str(), outlen);
      return;
    }
  }

  const char *error;
  int erroffset;
  static const pcre *re_lower =
    pcre_compile("/[a-z]/", 0, &error, &erroffset, nullptr);

  if (index(key, ':') == nullptr && !regex_match(re_lower, key)) {
    strncpy(normalizedKey, "ALL_CAPS_N_NUMBERS", outlen);
    return;
  }

  static const pcre *re_hash =
    pcre_compile("[a-f0-9]{8,}", 0, &error, &erroffset, nullptr);
  static const char *re_hash_replace = "{H}";
  static const pcre *re_number =
    pcre_compile("-?\\d+", 0, &error, &erroffset, nullptr);
  static const char *re_number_replace = "{N}";
  static const pcre *re_locale =
    pcre_compile("\\b[a-z][a-z]_[A-Z][A-Z]\\b", 0, &error, &erroffset, nullptr);
  static const char *re_locale_replace = "{L}";
  static const pcre *re_i18n =
    pcre_compile("^i{N}n", 0, &error, &erroffset, nullptr);
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

void SharedValueProfile::calcInd(const StringData *key,
                                 const APCHandle *variant) {
  keySize = key->size();
  variant->getStats(&var);
  totalSize = keySize + var.dataTotalSize;
}

void SharedValueProfile::addToGroup(SharedValueProfile *ind) {
  assert(isGroup);
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
  assert(isGroup);
  totalSize -= ind->totalSize;
  keySize -= ind->keySize;
  var.removeChildStats(&ind->var);
  keyCount--;
}

//////////////////////////////////////////////////////////////////////////////
// Definition of static members
#define MAX_KEY_LEN  120

std::atomic<int32_t> SharedStoreStats::s_keyCount(0);
std::atomic<int32_t> SharedStoreStats::s_keySize(0);
int32_t SharedStoreStats::s_variantCount = 0;
int64_t SharedStoreStats::s_dataSize = 0;
std::atomic<int64_t> SharedStoreStats::s_dataTotalSize(0);
int64_t SharedStoreStats::s_deleteSize = 0;
int64_t SharedStoreStats::s_replaceSize = 0;

std::atomic<int32_t> SharedStoreStats::s_addCount(0);
std::atomic<int32_t> SharedStoreStats::s_primeCount(0);
std::atomic<int32_t> SharedStoreStats::s_fromFileCount(0);
std::atomic<int32_t> SharedStoreStats::s_updateCount(0);
std::atomic<int32_t> SharedStoreStats::s_deleteCount(0);
std::atomic<int32_t> SharedStoreStats::s_expireCount(0);

int32_t SharedStoreStats::s_expireQueueSize = 0;
std::atomic<int64_t> SharedStoreStats::s_purgingTime(0);

ReadWriteMutex SharedStoreStats::s_rwlock;

SharedStoreStats::StatsMap SharedStoreStats::s_statsMap,
                           SharedStoreStats::s_detailMap;

//////////////////////////////////////////////////////////////////////////////
// Helpers for reporting and global aggregation

string SharedStoreStats::report_basic() {
  ostringstream out;
  out << "{\n";
  writeEntryInt(out, "Key_Count", s_keyCount, false, 1, true);
  writeEntryInt(out, "Size_Total", s_keySize + s_dataTotalSize, false, 1, true);
  writeEntryInt(out, "Size_Key", s_keySize, false, 1, true);
  writeEntryInt(out, "Size_Data", s_dataTotalSize, false, 1, true);
  writeEntryInt(out, "Add_Count", s_addCount, false, 1, true);
  writeEntryInt(out, "Prime_Count", s_primeCount, false, 1, true);
  writeEntryInt(out, "From_File_Count", s_fromFileCount, false, 1, true);
  writeEntryInt(out, "Update_Count", s_updateCount, false, 1, true);
  writeEntryInt(out, "Delete_Count", s_deleteCount, false, 1, true);
  writeEntryInt(out, "Expire_Count", s_expireCount, false, 1, true);
  writeEntryInt(out, "Expire_Queue_Size", s_expireQueueSize, false, 1, true);
  writeEntryInt(out, "Purging_Time", s_purgingTime, true, 1, true);
  out << "}\n";
  return out.str();
}

string SharedStoreStats::report_basic_flat() {
  ostringstream out;
  out << "{ " << "\"hphp.apc.size_total\":" << s_keySize + s_dataTotalSize
      << ", " << "\"hphp.apc.key_count\":" << s_keyCount
      << ", " << "\"hphp.apc.size_key\":" << s_keySize
      << ", " << "\"hphp.apc.size_data\":" << s_dataTotalSize
      << ", " << "\"hphp.apc.add_count\":" << s_addCount
      << ", " << "\"hphp.apc.prime_count\":" << s_primeCount
      << ", " << "\"hphp.apc.from_file_count\":" << s_fromFileCount
      << ", " << "\"hphp.apc.update_count\":" << s_updateCount
      << ", " << "\"hphp.apc.delete_count\":" << s_deleteCount
      << ", " << "\"hphp.apc.expire_count\":" << s_expireCount
      << ", " << "\"hphp.apc.expire_queue_size\":" << s_expireQueueSize
      << ", " << "\"hphp.apc.purging_time\":" << s_purgingTime
      << "}\n";
  return out.str();
}

string SharedStoreStats::report_keys() {
  ostringstream out;
  ReadLock l(s_rwlock);
  StatsMap::iterator iter;
  for (iter = s_statsMap.begin(); iter != s_statsMap.end(); ++iter) {
    assert(iter->second->isGroup);
    out << "{";
    writeEntryStr(out, "GroupName", iter->first);
    writeEntryInt(out, "TotalSize", iter->second->totalSize);
    writeEntryInt(out, "Count", iter->second->keyCount);
    writeEntryInt(out, "AvgTTL", iter->second->ttl);
    writeEntryInt(out, "SizeNoTTL", iter->second->sizeNoTTL);
    writeEntryInt(out, "KeySize", iter->second->keySize);
    writeEntryInt(out, "DataSize", iter->second->var.dataTotalSize, true);
    out << "}\n";
  }
  return out.str();
}

bool SharedStoreStats::snapshot(const char *filename, std::string& keySample) {
  std::ofstream out(filename);
  if (out.fail()) {
    return false;
  }
  char nkeySample[MAX_KEY_LEN + 1];
  if (keySample != "") {
    normalizeKey(keySample.c_str(), nkeySample, MAX_KEY_LEN);
    nkeySample[MAX_KEY_LEN] = '\0';
  }
  ReadLock l(s_rwlock);
  StatsMap::iterator iter;
  time_t now = time(nullptr);
  for (iter = s_detailMap.begin(); iter != s_detailMap.end();  ++iter) {
    assert(!iter->second->isGroup);
    if (keySample != "") {
      char nkey[MAX_KEY_LEN + 1];
      normalizeKey(iter->first, nkey, MAX_KEY_LEN);
      nkey[MAX_KEY_LEN] = '\0';
      if (strcmp(nkey, nkeySample) != 0) continue;
    }
    if (!iter->second->isValid) continue;
    out << "{";
    writeEntryStr(out, "KeyName", iter->first);
    writeEntryInt(out, "TotalSize", iter->second->totalSize);
    writeEntryInt(out, "Prime", (int64_t)iter->second->isPrime);
    writeEntryInt(out, "TTL", iter->second->ttl);
    writeEntryInt(out, "KeySize", iter->second->keySize);
    writeEntryInt(out, "DataSize", iter->second->var.dataTotalSize);
    writeEntryInt(out, "StoreCount", iter->second->storeCount);
    writeEntryInt(out, "SinceLastStore", now - iter->second->lastStoreTime);
    writeEntryInt(out, "DeleteCount", iter->second->deleteCount);
    if (iter->second->deleteCount > 0) {
      writeEntryInt(out, "SinceLastDelete",
                    now - iter->second->lastDeleteTime);
    }
    if (RuntimeOption::EnableAPCFetchStats) {
      writeEntryInt(out, "FetchCount", iter->second->fetchCount);
      if (iter->second->fetchCount > 0) {
        writeEntryInt(out, "SinceLastFetch",
                      now - iter->second->lastFetchTime);
      }
    }
    writeEntryInt(out, "VariantCount", iter->second->var.variantCount);
    writeEntryInt(out, "UserDataSize", iter->second->var.dataSize, true);
    out << "}\n";
  }
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

void SharedStoreStats::addDirect(int32_t keySize, int32_t dataTotal, bool prime,
                                 bool file) {
  s_keyCount.fetch_add(1, std::memory_order_relaxed);
  s_keySize.fetch_add(keySize, std::memory_order_relaxed);
  s_dataTotalSize.fetch_add((int64_t)dataTotal, std::memory_order_relaxed);
  s_addCount.fetch_add(1, std::memory_order_relaxed);
  if (prime) {
    s_primeCount.fetch_add(1, std::memory_order_relaxed);
  }
  if (file) {
    s_fromFileCount.fetch_add(1, std::memory_order_relaxed);
  }
}

void SharedStoreStats::removeDirect(int32_t keySize, int32_t dataTotal, bool exp) {
  s_keyCount.fetch_sub(1, std::memory_order_relaxed);
  s_keySize.fetch_sub(keySize, std::memory_order_relaxed);
  s_dataTotalSize.fetch_sub((int64_t)dataTotal, std::memory_order_relaxed);
  if (exp) {
    s_expireCount.fetch_add(1, std::memory_order_relaxed);
  } else {
    s_deleteCount.fetch_add(1, std::memory_order_relaxed);
  }
}

void SharedStoreStats::updateDirect(int32_t dataTotalOld, int32_t dataTotalNew) {
  s_dataTotalSize.fetch_sub((int64_t)dataTotalOld, std::memory_order_relaxed);
  s_dataTotalSize.fetch_add((int64_t)dataTotalNew, std::memory_order_relaxed);
  s_updateCount.fetch_add(1, std::memory_order_relaxed);
}

void SharedStoreStats::addPurgingTime(int64_t purgingTime) {
  s_purgingTime.fetch_add(purgingTime, std::memory_order_relaxed);
}

void SharedStoreStats::onDelete(const StringData *key, const APCHandle *var,
                                bool replace, bool noTTL) {
  char normalizedKey[MAX_KEY_LEN + 1];

  if (RuntimeOption::EnableAPCSizeGroup) {
    normalizeKey(key->data(), normalizedKey, MAX_KEY_LEN);
    normalizedKey[MAX_KEY_LEN] = '\0';
  }

  SharedValueProfile svpTemp;
  // Calculate size of the variant
  svpTemp.calcInd(key, var);

  ReadLock l(s_rwlock);

  if (RuntimeOption::EnableAPCSizeDetail && !replace) {
    StatsMap::const_accessor cacc;
    if (s_detailMap.find(cacc, (char*)key->data())) {
      SharedValueProfile *svp = cacc->second;
      assert(svp->isValid);
      svp->isValid = false;
      svp->deleteCount++;
      svp->lastDeleteTime = time(nullptr);
    }
  }

  if (RuntimeOption::EnableAPCSizeGroup) {
    StatsMap::const_accessor cacc;
    if (s_statsMap.find(cacc, normalizedKey)) {
      SharedValueProfile *group = cacc->second;
      group->removeFromGroup(&svpTemp);
      if (noTTL) {
        group->sizeNoTTL -= svpTemp.totalSize;
      }
    }
  }
}

void SharedStoreStats::onGet(const StringData *key, const APCHandle *var) {
  ReadLock l(s_rwlock);
  StatsMap::const_accessor cacc;
  if (s_detailMap.find(cacc, (char*)key->data())) {
    SharedValueProfile *svpInd = cacc->second;
    svpInd->lastFetchTime = time(nullptr);
    svpInd->fetchCount++;
  }
}

void SharedStoreStats::onStore(const StringData *key, const APCHandle *var,
                               int64_t ttl, bool prime) {
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

  ReadLock l(s_rwlock);

  if (RuntimeOption::EnableAPCSizeGroup) {
    SharedValueProfile *group;
    StatsMap::const_accessor cacc;
    StatsMap::accessor acc;
    if (s_statsMap.find(cacc, normalizedKey)) {
      group = cacc->second;
    } else {
      cacc.release();
      group = new SharedValueProfile(normalizedKey);
      if (s_statsMap.insert(acc, group->key)) {
        group->isGroup = true;
        group->keyCount = 0;
        acc->second = group;
      } else {
        // already there
        delete group;
        group = acc->second;
      }
    }
    group->addToGroup(svpInd);
    if (ttl == 0) {
      group->sizeNoTTL += svpInd->totalSize;
    }
  }

  if (RuntimeOption::EnableAPCSizeDetail) {
    StatsMap::accessor acc;
    if (s_detailMap.insert(acc, svpInd->key)) {
      acc->second = svpInd;
      if (prime) {
        svpInd->isPrime = true;
      }
    } else {
      SharedValueProfile *existing = acc->second;
      // update size but keep other stats
      existing->totalSize = svpInd->totalSize;
      existing->keySize = svpInd->keySize;
      existing->var = svpInd->var;
      delete svpInd;
      svpInd = existing;
    }
    svpInd->isValid = true;
    svpInd->storeCount++;
    svpInd->lastStoreTime = time(nullptr);
  } else {
    delete svpInd;
  }
}

///////////////////////////////////////////////////////////////////////////////
}
