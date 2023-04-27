/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

class OPTIONS_NAME : public McrouterOptionsBase {
 public:
#define MCROUTER_STRING_MAP std::unordered_map<std::string, std::string>
#define MCROUTER_OPTION(_type, _name, _def, _l, _s, _d, _T) _type _name{_def};
#define MCROUTER_OPTION_STRING(_n, _f, _l, _s, _d) \
  MCROUTER_OPTION(std::string, _n, _f, _l, _s, _d, string)
#define MCROUTER_OPTION_INTEGER(_t, _n, _f, _l, _s, _d) \
  MCROUTER_OPTION(_t, _n, _f, _l, _s, _d, integer)
#define MCROUTER_OPTION_DOUBLE(_t, _n, _f, _l, _s, _d) \
  MCROUTER_OPTION(_t, _n, _f, _l, _s, _d, double_precision)
#define MCROUTER_OPTION_TOGGLE(_n, _f, _l, _s, _d) \
  MCROUTER_OPTION(bool, _n, _f, _l, _s, _d, toggle)
#define MCROUTER_OPTION_STRING_MAP(_n, _l, _s, _d) \
  MCROUTER_OPTION(MCROUTER_STRING_MAP, _n, , _l, _s, _d, string_map)
#define MCROUTER_OPTION_OTHER(_t, _n, _f, _l, _s, _d) \
  MCROUTER_OPTION(_t, _n, _f, _l, _s, _d, other)

#include OPTIONS_FILE

  OPTIONS_NAME() = default;

  OPTIONS_NAME(OPTIONS_NAME&&) = default;
  OPTIONS_NAME& operator=(OPTIONS_NAME&&) = default;

  OPTIONS_NAME clone() const {
    return *this;
  }

  void forEach(
      std::function<
          void(const std::string&, McrouterOptionData::Type, const boost::any&)>
          f) const override {
#undef MCROUTER_OPTION
#define MCROUTER_OPTION(_type, _name, _f, _l, _s, _d, _Type) \
  f(#_name,                                                  \
    McrouterOptionData::Type::_Type,                         \
    boost::any(const_cast<_type*>(&_name)));

#include OPTIONS_FILE
  }

  static std::vector<McrouterOptionData> getOptionData() {
    std::vector<McrouterOptionData> ret;
    std::string current_group;

#undef MCROUTER_OPTION
#define MCROUTER_OPTION(_type, _name, _default, _lopt, _sopt, _doc, _Type) \
  {                                                                        \
    McrouterOptionData opt;                                                \
    opt.type = McrouterOptionData::Type::_Type;                            \
    opt.name = #_name;                                                     \
    opt.group = current_group;                                             \
    opt.default_value = #_default;                                         \
    opt.long_option = _lopt;                                               \
    opt.short_option = _sopt;                                              \
    opt.docstring = _doc;                                                  \
    ret.push_back(opt);                                                    \
  }

#undef MCROUTER_OPTION_GROUP
#define MCROUTER_OPTION_GROUP(_name) current_group = _name;

#include OPTIONS_FILE

    return ret;
  }

 private:
  OPTIONS_NAME(const OPTIONS_NAME&) = default;
  OPTIONS_NAME& operator=(const OPTIONS_NAME&) = default;
};

#undef MCROUTER_OPTION_STRING
#undef MCROUTER_OPTION_INTEGER
#undef MCROUTER_OPTION_DOUBLE
#undef MCROUTER_OPTION_TOGGLE
#undef MCROUTER_OPTION_STRING_MAP
#undef MCROUTER_OPTION_OTHER
#undef MCROUTER_OPTION
#undef MCROUTER_STRING_MAP
