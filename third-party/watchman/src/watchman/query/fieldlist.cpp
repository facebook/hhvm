/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/CommandRegistry.h"
#include "watchman/Errors.h"
#include "watchman/query/FileResult.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryContext.h"
#include "watchman/watchman_time.h"

namespace watchman {

namespace {

std::optional<json_ref> make_name(FileResult* file, const QueryContext* ctx) {
  return w_string_to_json(ctx->computeWholeName(file));
}

std::optional<json_ref> make_symlink(FileResult* file, const QueryContext*) {
  auto target = file->readLink();
  if (!target.has_value()) {
    return std::nullopt;
  }
  if (std::holds_alternative<NotSymlink>(*target)) {
    return json_null();
  } else {
    return w_string_to_json(std::get<w_string>(*target));
  }
}

std::optional<json_ref> make_sha1_hex(FileResult* file, const QueryContext*) {
  try {
    auto hash = file->getContentSha1();
    if (!hash.has_value()) {
      // Need to load it still
      return std::nullopt;
    }
    char buf[40];
    static const char* hexDigit = "0123456789abcdef";
    for (size_t i = 0; i < hash->size(); ++i) {
      auto& digit = (*hash)[i];
      buf[(i * 2) + 0] = hexDigit[digit >> 4];
      buf[(i * 2) + 1] = hexDigit[digit & 0xf];
    }
    return w_string_to_json(w_string(buf, sizeof(buf), W_STRING_UNICODE));
  } catch (const std::system_error& exc) {
    auto errcode = exc.code();
    if (errcode == error_code::no_such_file_or_directory ||
        errcode == error_code::is_a_directory) {
      // Deleted files, or (currently existing) directories have no hash
      return json_null();
    }
    // We'll report the error wrapped up in an object so that it can be
    // distinguished from a valid hash result.
    return json_object(
        {{"error", w_string_to_json(w_string(exc.what(), W_STRING_UNICODE))}});
  } catch (const std::exception& exc) {
    // We'll report the error wrapped up in an object so that it can be
    // distinguished from a valid hash result.
    return json_object(
        {{"error", w_string_to_json(w_string(exc.what(), W_STRING_UNICODE))}});
  }
}

std::optional<json_ref> make_size(FileResult* file, const QueryContext*) {
  auto size = file->size();
  if (!size.has_value()) {
    return std::nullopt;
  }
  return json_integer(size.value());
}

std::optional<json_ref> make_exists(FileResult* file, const QueryContext*) {
  auto exists = file->exists();
  if (!exists.has_value()) {
    return std::nullopt;
  }
  return json_boolean(exists.value());
}

std::optional<json_ref> make_new(FileResult* file, const QueryContext* ctx) {
  bool is_new = false;

  auto* since_clock = std::get_if<QuerySince::Clock>(&ctx->since.since);
  if (since_clock && since_clock->is_fresh_instance) {
    is_new = true;
  } else {
    auto ctime = file->ctime();
    if (!ctime.has_value()) {
      // Reconsider this one later
      return std::nullopt;
    }
    if (since_clock) {
      is_new = ctime->ticks > since_clock->ticks;
    } else {
      auto& since_ts = std::get<QuerySince::Timestamp>(ctx->since.since);
      is_new = since_ts.time > ctime->timestamp;
    }
  }

  return json_boolean(is_new);
}

#define MAKE_CLOCK_FIELD(name, member)                      \
  static std::optional<json_ref> make_##name(               \
      FileResult* file, const QueryContext* ctx) {          \
    char buf[128];                                          \
    auto clock = file->member();                            \
    if (!clock.has_value()) {                               \
      /* need to load data */                               \
      return std::nullopt;                                  \
    }                                                       \
    if (clock_id_string(                                    \
            ctx->clockAtStartOfQuery.position().rootNumber, \
            clock->ticks,                                   \
            buf,                                            \
            sizeof(buf))) {                                 \
      return typed_string_to_json(buf, W_STRING_UNICODE);   \
    }                                                       \
    return json_null();                                     \
  }
MAKE_CLOCK_FIELD(cclock, ctime)
MAKE_CLOCK_FIELD(oclock, otime)

// Note: our JSON library supports 64-bit integers, but this may
// pose a compatibility issue for others.  We'll see if anyone
// runs into an issue and deal with it then...
static_assert(
    sizeof(json_int_t) >= sizeof(time_t),
    "json_int_t isn't large enough to hold a time_t");

#define MAKE_INT_FIELD(name, member)           \
  static std::optional<json_ref> make_##name(  \
      FileResult* file, const QueryContext*) { \
    auto stat = file->stat();                  \
    if (!stat.has_value()) {                   \
      /* need to load data */                  \
      return std::nullopt;                     \
    }                                          \
    return json_integer(stat->member);         \
  }

#define MAKE_TIME_INT_FIELD(name, member, scale)                  \
  static std::optional<json_ref> make_##name(                     \
      FileResult* file, const QueryContext*) {                    \
    auto spec = file->member();                                   \
    if (!spec.has_value()) {                                      \
      /* need to load data */                                     \
      return std::nullopt;                                        \
    }                                                             \
    return json_integer(                                          \
        ((int64_t)spec->tv_sec * scale) +                         \
        ((int64_t)spec->tv_nsec * scale / WATCHMAN_NSEC_IN_SEC)); \
  }

#define MAKE_TIME_DOUBLE_FIELD(name, member)               \
  static std::optional<json_ref> make_##name(              \
      FileResult* file, const QueryContext*) {             \
    auto spec = file->member();                            \
    if (!spec.has_value()) {                               \
      /* need to load data */                              \
      return std::nullopt;                                 \
    }                                                      \
    return json_real(spec->tv_sec + 1e-9 * spec->tv_nsec); \
  }

/* For each type (e.g. "m"), define fields
 * - mtime: mtime in seconds
 * - mtime_ms: mtime in milliseconds
 * - mtime_us: mtime in microseconds
 * - mtime_ns: mtime in nanoseconds
 * - mtime_f: mtime as a double
 */
#define MAKE_TIME_FIELDS(type, member)                           \
  MAKE_TIME_INT_FIELD(type##time, member, 1)                     \
  MAKE_TIME_INT_FIELD(type##time_ms, member, 1000)               \
  MAKE_TIME_INT_FIELD(type##time_us, member, 1000 * 1000)        \
  MAKE_TIME_INT_FIELD(type##time_ns, member, 1000 * 1000 * 1000) \
  MAKE_TIME_DOUBLE_FIELD(type##time_f, member)

MAKE_INT_FIELD(mode, mode)
MAKE_INT_FIELD(uid, uid)
MAKE_INT_FIELD(gid, gid)
MAKE_TIME_FIELDS(a, accessedTime)
MAKE_TIME_FIELDS(m, modifiedTime)
MAKE_TIME_FIELDS(c, changedTime)
MAKE_INT_FIELD(ino, ino)
MAKE_INT_FIELD(dev, dev)
MAKE_INT_FIELD(nlink, nlink)

// clang-format off
#define MAKE_TIME_FIELD_DEFS(type) \
  { #type "time", make_##type##time}, \
  { #type "time_ms", make_##type##time_ms},\
  { #type "time_us", make_##type##time_us}, \
  { #type "time_ns", make_##type##time_ns}, \
  { #type "time_f", make_##type##time_f}
// clang-format on

std::optional<json_ref> make_type_field(FileResult* file, const QueryContext*) {
  auto dtype = file->dtype();
  if (dtype.has_value()) {
    switch (*dtype) {
      case DType::Regular:
        return typed_string_to_json("f", W_STRING_UNICODE);
      case DType::Dir:
        return typed_string_to_json("d", W_STRING_UNICODE);
      case DType::Symlink:
        return typed_string_to_json("l", W_STRING_UNICODE);
      case DType::Block:
        return typed_string_to_json("b", W_STRING_UNICODE);
      case DType::Char:
        return typed_string_to_json("c", W_STRING_UNICODE);
      case DType::Fifo:
        return typed_string_to_json("p", W_STRING_UNICODE);
      case DType::Socket:
        return typed_string_to_json("s", W_STRING_UNICODE);
      case DType::Whiteout:
        // Whiteout shouldn't generally be visible to userspace,
        // and we don't have a defined letter code for it, so
        // treat it as "who knows!?"
        return typed_string_to_json("?", W_STRING_UNICODE);
      case DType::Unknown:
      default:
          // Not enough info; fall through and use the full stat data
          ;
    }
  }

  // Bias towards the more common file types first
  auto optionalStat = file->stat();
  if (!optionalStat.has_value()) {
    return std::nullopt;
  }

  auto stat = optionalStat.value();
  if (stat.isFile()) {
    return typed_string_to_json("f", W_STRING_UNICODE);
  }
  if (stat.isDir()) {
    return typed_string_to_json("d", W_STRING_UNICODE);
  }
  if (stat.isSymlink()) {
    return typed_string_to_json("l", W_STRING_UNICODE);
  }
#ifndef _WIN32
  if (S_ISBLK(stat.mode)) {
    return typed_string_to_json("b", W_STRING_UNICODE);
  }
  if (S_ISCHR(stat.mode)) {
    return typed_string_to_json("c", W_STRING_UNICODE);
  }
  if (S_ISFIFO(stat.mode)) {
    return typed_string_to_json("p", W_STRING_UNICODE);
  }
  if (S_ISSOCK(stat.mode)) {
    return typed_string_to_json("s", W_STRING_UNICODE);
  }
#endif
#ifdef S_ISDOOR
  if (S_ISDOOR(stat.mode)) {
    return typed_string_to_json("D", W_STRING_UNICODE);
  }
#endif
  return typed_string_to_json("?", W_STRING_UNICODE);
}

// Helper to construct the list of field defs
std::unordered_map<w_string, QueryFieldRenderer> build_defs() {
  struct {
    const char* name;
    std::optional<json_ref> (*make)(FileResult* file, const QueryContext* ctx);
  } defs[] = {
      {"name", make_name},
      {"symlink_target", make_symlink},
      {"exists", make_exists},
      {"size", make_size},
      {"mode", make_mode},
      {"uid", make_uid},
      {"gid", make_gid},
      MAKE_TIME_FIELD_DEFS(a),
      MAKE_TIME_FIELD_DEFS(m),
      MAKE_TIME_FIELD_DEFS(c),
      {"ino", make_ino},
      {"dev", make_dev},
      {"nlink", make_nlink},
      {"new", make_new},
      {"oclock", make_oclock},
      {"cclock", make_cclock},
      {"type", make_type_field},
      {"content.sha1hex", make_sha1_hex},
  };
  std::unordered_map<w_string, QueryFieldRenderer> map;
  for (auto& def : defs) {
    w_string name(def.name, W_STRING_UNICODE);
    map.emplace(name, QueryFieldRenderer{name, def.make});
  }

  return map;
}

// Meyers singleton to avoid SIOF wrt. static constructors in this module
// and the order that w_ctor_fn callbacks are dispatched.
std::unordered_map<w_string, QueryFieldRenderer>& field_defs() {
  static std::unordered_map<w_string, QueryFieldRenderer> map(build_defs());
  return map;
}

} // namespace

void QueryFieldList::add(const w_string& name) {
  auto& defs = field_defs();
  auto it = defs.find(name);
  if (it == defs.end()) {
    QueryParseError::throwf("unknown field name '{}'", name);
  }
  this->push_back(&it->second);
}

json_ref field_list_to_json_name_array(const QueryFieldList& fieldList) {
  std::vector<json_ref> templ;
  templ.reserve(fieldList.size());

  for (auto& f : fieldList) {
    templ.push_back(w_string_to_json(f->name));
  }

  return json_array(std::move(templ));
}

void parse_field_list(
    const std::optional<json_ref>& maybe_field_list,
    QueryFieldList* selected) {
  selected->clear();

  json_ref field_list = maybe_field_list
      ? *maybe_field_list
      : json_array(
            {typed_string_to_json("name", W_STRING_UNICODE),
             typed_string_to_json("exists", W_STRING_UNICODE),
             typed_string_to_json("new", W_STRING_UNICODE),
             typed_string_to_json("size", W_STRING_UNICODE),
             typed_string_to_json("mode", W_STRING_UNICODE)});

  if (!field_list.isArray()) {
    throw QueryParseError("field list must be an array of strings");
  }

  for (auto& jname : field_list.array()) {
    if (!jname.isString()) {
      throw QueryParseError("field list must be an array of strings");
    }

    auto name = json_to_w_string(jname);
    selected->add(name);
  }
}

namespace {
struct register_field_capabilities {
  register_field_capabilities() {
    for (auto& it : field_defs()) {
      char capname[128];
      snprintf(capname, sizeof(capname), "field-%s", it.first.c_str());
      capability_register(capname);
    }
  }
} reg;
} // namespace

} // namespace watchman
