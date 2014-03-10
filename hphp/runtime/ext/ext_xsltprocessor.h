
#ifndef incl_EXT_XSLTPROCESSOR_H_
#define incl_EXT_XSLTPROCESSOR_H_

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/extensions.h>
#include <libxslt/variables.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/security.h>
#include <libexslt/exslt.h>

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_XSL_SECPREF_NONE;
extern const int64_t k_XSL_SECPREF_READ_FILE;
extern const int64_t k_XSL_SECPREF_WRITE_FILE;
extern const int64_t k_XSL_SECPREF_CREATE_DIRECTORY;
extern const int64_t k_XSL_SECPREF_READ_NETWORK;
extern const int64_t k_XSL_SECPREF_WRITE_NETWORK;
extern const int64_t k_XSL_SECPREF_DEFAULT;

///////////////////////////////////////////////////////////////////////////////
// class XSLTProcessor

FORWARD_DECLARE_CLASS(XSLTProcessor);
class c_XSLTProcessor : public ExtObjectData, public Sweepable {
 public:
  DECLARE_CLASS(XSLTProcessor)

  // need to implement
  public: c_XSLTProcessor(Class* cls = c_XSLTProcessor::classof());
  public: ~c_XSLTProcessor();
  public: void t___construct();
  public: Variant t_getparameter(const String& namespaceURI,
                                 const String& localName);
  public: int64_t t_getsecurityprefs();
  public: bool t_hasexsltsupport();
  public: void t_importstylesheet(const Object& stylesheet);
  public: void t_registerphpfunctions(const Variant& funcs = null_variant);
  public: bool t_removeparameter(const String& namespaceURI,
                                 const String& localName);
  public: bool t_setparameter(const String& namespaceURI,
                              const Variant& localName,
                              const Variant& value = null_variant);
  public: bool t_setprofiling(const String& filename);
  public: int64_t t_setsecurityprefs(int64_t securityPrefs);
  public: Variant t_transformtodoc(const Object& doc);
  public: Variant t_transformtouri(const Object& doc, const String& uri);
  public: Variant t_transformtoxml(const Object& doc);

  public:
    xsltStylesheetPtr m_stylesheet;
    xmlDocPtr m_doc;
    Array m_params;
    int m_secprefs;
    int m_registerPhpFunctions;
    Array m_registered_phpfunctions;
    String m_profile;

  private:
    xmlDocPtr apply_stylesheet();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_EXT_XSLTPROCESSOR_H_
