#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/ext/asio/asio-external-thread-event.h"

#include <memory>

#include "mcrouter/config.h" // @nolint
#include "mcrouter/options.h" // @nolint
#include "mcrouter/McrouterClient.h" // @nolint
#include "mcrouter/McrouterInstance.h" // @nolint

namespace mc  = facebook::memcache;
namespace mcr = facebook::memcache::mcrouter;

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

const StaticString
  s_MCRouter("MCRouter"),
  s_MCRouterException("MCRouterException"),
  s_MCRouterOptionException("MCRouterOptionException"),
  s_option("option"),
  s_value("value"),
  s_error("error");

static Class* c_MCRouterException = nullptr;
static Object mcr_getException(const std::string& message,
                               mc_op_t op = mc_op_unknown,
                               mc_res_t reply = mc_res_unknown,
                               const std::string& key = "") {
  if (!c_MCRouterException) {
    c_MCRouterException = Unit::lookupClass(s_MCRouterException.get());
    assert(c_MCRouterException);
  }

  Object obj{c_MCRouterException};
  TypedValue ret;
  g_context->invokeFunc(
    &ret,
    c_MCRouterException->getCtor(),
    make_packed_array(message, (int64_t)op, (int64_t)reply, key),
    obj.get());
  tvRefcountedDecRef(&ret);
  return obj;
}

static Class* c_MCRouterOptionException = nullptr;
static Object mcr_getOptionException(
  const std::vector<mc::McrouterOptionError>& errors) {
  if (!c_MCRouterOptionException) {
    c_MCRouterOptionException =
      Unit::lookupClass(s_MCRouterOptionException.get());
    assert(c_MCRouterOptionException);
  }

  Array errorArray = Array::Create();
  for (auto err : errors) {
    Array e;
    e.set(s_option, String(err.requestedName));
    e.set(s_value, String(err.requestedValue));
    e.set(s_error, String(err.errorMsg));
    errorArray.append(e);
  }

  Object obj{c_MCRouterOptionException};
  TypedValue ret;
  g_context->invokeFunc(
    &ret,
    c_MCRouterOptionException->getCtor(),
    make_packed_array(errorArray),
    obj.get());
  return obj;
}

/////////////////////////////////////////////////////////////////////////////

class MCRouter {
 public:
  MCRouter() {}
  MCRouter& operator=(const MCRouter& str) = delete;

  static void onReply(mcr::mcrouter_msg_t*, void*);
  static void onCancel(void*, void*);

  void send(const mcr::mcrouter_msg_t& msg) {
    m_client->send(&msg, 1);
  }

  void init(const Array& options, const String& pid) {
    mc::McrouterOptions opts;
    parseOptions(opts, options);

    mcr::McrouterInstance* router;
    if (pid.empty()) {
      /* TODO(t7574316): clean up */
#ifdef HPHP_OSS
      router = mcr::McrouterInstance::createTransient(opts);
#else
      m_transientRouter = mcr::McrouterInstance::create(opts.clone());
      router = m_transientRouter.get();
#endif  /* HPHP_OSS */
    } else {
      router = mcr::McrouterInstance::init(pid.toCppString(), opts);
    }

    if (!router) {
      throw mcr_getException("Unable to initialize MCRouter instance");
    }

    m_client = router->createClient(
      {onReply,onCancel,nullptr},
      this,
      0);

    if (!m_client) {
      throw mcr_getException("Unable to initilize MCRouterClient instance");
    }
  }

  Object issue(mcr::mcrouter_msg_t& msg);

 private:
  std::shared_ptr<mcr::McrouterInstance> m_transientRouter;
  mcr::McrouterClient::Pointer m_client;

  void parseOptions(mc::McrouterOptions& opts, const Array& options) {
#ifdef HPHP_OSS
    // Change defaults for these since they make assumptions about the system
    opts.asynclog_disable = true;
    opts.async_spool = "";
    opts.stats_logging_interval = 0;
    opts.stats_root = "";
#else
    // Default this to true since this is a new interface
    // and it allows our tests to work inside or out
    opts.use_new_configs = true;
#endif

    std::unordered_map<std::string, std::string> dict;
    for (ArrayIter iter(options); iter; ++iter) {
      auto key = iter.first().toString().toCppString();
      auto val = iter.second();
      if (val.isBoolean()) {
        // false -> toString() == "" which will fail foll::to<bool>
        dict[key] = val.toBoolean() ? "1" : "0";
      } else {
        dict[key] = val.toString().toCppString();
      }
    }

    auto errors = opts.updateFromDict(dict);
    if (!errors.empty()) {
      throw mcr_getOptionException(errors);
    }
  }
};

class MCRouterResult : public AsioExternalThreadEvent {
 public:
  MCRouterResult(MCRouter* router, mcr::mcrouter_msg_t& msg) {
    m_result.m_type = KindOfNull;
    msg.context = this;
    router->send(msg);
  }

  void unserialize(Cell& c) override {
    if (!m_exception.empty()) {
      throw mcr_getException(m_exception, m_op, m_replyCode, m_key);
    }
    if ((m_result.m_type == KindOfString) && !m_result.m_data.pstr) {
      // Deferred string init, see below
      m_result.m_data.pstr = StringData::Make(
        m_stringResult.c_str(), m_stringResult.size(), CopyString);
      m_stringResult.clear();
    }
    cellDup(m_result, c);
  }

  void result(const mcr::mcrouter_msg_t* msg) {
    if (msg->reply.isError()) {
      setResultException(msg);
    } else {
      switch (msg->req->op) {
        case mc_op_add:
        case mc_op_set:
        case mc_op_replace:
        case mc_op_prepend:
        case mc_op_append:
        case mc_op_flushall:
          if (!msg->reply.isStored()) {
            setResultException(msg);
            break;
          }
          break;

        case mc_op_delete:
          if (msg->reply.result() != mc_res_deleted) {
            setResultException(msg);
            break;
          }
          break;

        case mc_op_incr:
        case mc_op_decr:
          if (!msg->reply.isStored()) {
            setResultException(msg);
            break;
          }
          m_result.m_type = KindOfInt64;
          m_result.m_data.num = msg->reply.delta();
          break;

        case mc_op_get:
          if (msg->reply.isMiss()) {
            setResultException(msg);
            break;
          }
          /* fallthrough */
        case mc_op_version:
          m_result.m_type = KindOfString;
          m_result.m_data.pstr = nullptr;
          // We're in the wrong thread for making a StringData
          // so stash it in a std::string until we get to unserialize
          m_stringResult = std::string((char*)msg->reply.value().data(),
                                       msg->reply.value().length());
          break;

        default:
          always_assert(false);
      }
    }
    markAsFinished();
  }

 private:

  // Store the important parts of the exception in non-thread vars
  // to bubble up during unserialize
  void setResultException(const mcr::mcrouter_msg_t* msg) {
    m_op = msg->req->op;
    m_replyCode = msg->reply.result();
    m_exception  = mc_op_to_string(m_op);
    m_exception += " failed with result ";
    m_exception += mc_res_to_string(m_replyCode);
    if (msg->reply.value().length() > 0) {
      m_exception += ": ";
      m_exception += std::string((char*)msg->reply.value().data(),
                                 msg->reply.value().length());
    }
    m_key = std::string(msg->req->key.str, msg->req->key.len);
  }

  Cell m_result;

  // Deferred string result
  std::string m_stringResult;

  // Deferred exception data
  mc_op_t m_op;
  mc_res_t m_replyCode;
  std::string m_exception, m_key;
};

void MCRouter::onReply(mcr::mcrouter_msg_t* msg, void* router) {
  ((MCRouterResult*)msg->context)->result(msg);
}

void MCRouter::onCancel(void* request, void* router) {
  ((MCRouterResult*)request)->cancel();
}

Object MCRouter::issue(mcr::mcrouter_msg_t& msg) {
  auto ev = new MCRouterResult(this, msg);
  try {
    return ev->getWaitHandle();
  } catch (...) {
    assert(false);
    ev->abandon();
    throw;
  }
}

/////////////////////////////////////////////////////////////////////////////

static void HHVM_METHOD(MCRouter, __construct,
                        const Array& opts, const String& pid) {
  Native::data<MCRouter>(this_)->init(opts, pid);
}

template<mc_op_t op>
static Object mcr_str(ObjectData* this_, const String& key) {
  mcr::mcrouter_msg_t msg;
  msg.req = mc_msg_new_with_key_full(key.c_str(), key.size());
  msg.req->op = op;
  return Native::data<MCRouter>(this_)->issue(msg);
}

template<mc_op_t op>
static Object mcr_set(ObjectData* this_,
                      const String& key, const String& val,
                      int64_t flags, int64_t expiration) {
  mcr::mcrouter_msg_t msg;
  msg.req = mc_msg_new_with_key_and_value_full(key.c_str(), key.size(),
                                               val.c_str(), val.size());
  msg.req->op = op;
  msg.req->flags = flags;
  msg.req->exptime = expiration;
  return Native::data<MCRouter>(this_)->issue(msg);
}

template<mc_op_t op>
static Object mcr_aprepend(ObjectData* this_,
                           const String& key, const String& val) {
  mcr::mcrouter_msg_t msg;
  msg.req = mc_msg_new_with_key_and_value_full(key.c_str(), key.size(),
                                               val.c_str(), val.size());
  msg.req->op = op;
  return Native::data<MCRouter>(this_)->issue(msg);
}

template<mc_op_t op>
static Object mcr_str_delta(ObjectData* this_,
                          const String& key, int64_t val) {
  mcr::mcrouter_msg_t msg;
  msg.req = mc_msg_new_with_key_full(key.c_str(), key.size());
  msg.req->delta = val;
  msg.req->op = op;
  return Native::data<MCRouter>(this_)->issue(msg);
}

template<mc_op_t op>
static Object mcr_int(ObjectData* this_, int64_t val) {
  mcr::mcrouter_msg_t msg;
  msg.req = mc_msg_new(0);
  msg.req->number = val;
  msg.req->op = op;
  return Native::data<MCRouter>(this_)->issue(msg);
}


template<mc_op_t op>
static Object mcr_void(ObjectData* this_) {
  mcr::mcrouter_msg_t msg;
  msg.req = mc_msg_new(0);
  msg.req->op = mc_op_version;
  return Native::data<MCRouter>(this_)->issue(msg);
}

/////////////////////////////////////////////////////////////////////////////

static String HHVM_STATIC_METHOD(MCRouter, getOpName, int64_t op) {
  auto name = mc_op_to_string((mc_op_t)op);
  if (!name) {
    std::string msg = "Unknown mc_op_* value: ";
    msg += op;
    throw mcr_getException(msg, (mc_op_t)op);
  }
  return name;
}

static String HHVM_STATIC_METHOD(MCRouter, getResultName, int64_t res) {
  auto name = mc_res_to_string((mc_res_t)res);
  if (!name) {
    std::string msg = "Unknown mc_res_* value: ";
    msg += res;
    throw mcr_getException(msg, mc_op_unknown, (mc_res_t)res);
  }
  return name;
}

/////////////////////////////////////////////////////////////////////////////

class MCRouterExtension : public Extension {
 public:
  MCRouterExtension(): Extension("mcrouter", "1.0.0") {}

  void moduleInit() override {
    HHVM_ME(MCRouter, __construct);

    HHVM_NAMED_ME(MCRouter, get, mcr_str<mc_op_get>);

    HHVM_NAMED_ME(MCRouter, add, mcr_set<mc_op_add>);
    HHVM_NAMED_ME(MCRouter, set, mcr_set<mc_op_set>);
    HHVM_NAMED_ME(MCRouter, replace, mcr_set<mc_op_replace>);
    HHVM_NAMED_ME(MCRouter, prepend, mcr_aprepend<mc_op_prepend>);
    HHVM_NAMED_ME(MCRouter, append, mcr_aprepend<mc_op_append>);

    HHVM_NAMED_ME(MCRouter, incr, mcr_str_delta<mc_op_incr>);
    HHVM_NAMED_ME(MCRouter, decr, mcr_str_delta<mc_op_decr>);

    HHVM_NAMED_ME(MCRouter, del, mcr_str<mc_op_delete>);
    HHVM_NAMED_ME(MCRouter, flushAll, mcr_int<mc_op_flushall>);

    HHVM_NAMED_ME(MCRouter, version, mcr_void<mc_op_version>);

    Native::registerNativeDataInfo<MCRouter>(s_MCRouter.get());

    HHVM_STATIC_ME(MCRouter, getOpName);
    HHVM_STATIC_ME(MCRouter, getResultName);

    std::string opname("mc_op_");
    for (int i = 0; i < mc_nops; ++i) {
      std::string name;
      name = opname + mc_op_to_string((mc_op_t)i);
      // mcrouter defines op names as foo-bar,
      // but PHP wants constants like foo_bar
      for (int j = opname.size(); j < name.size(); ++j) {
        if (name[j] == '-') {
          name[j] = '_';
        }
      }
      Native::registerClassConstant<KindOfInt64>(
        s_MCRouter.get(),
        makeStaticString(name),
        i);
    }
    for (int i = 0; i < mc_nres; ++i) {
      Native::registerClassConstant<KindOfInt64>(
        s_MCRouter.get(),
        makeStaticString(mc_res_to_string((mc_res_t)i)),
        i);
    }

    loadSystemlib();
  }
} s_mcrouter_extension;

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
