#ifndef incl_HPHP_PDO_PGSQL_STATEMENT_H_
#define incl_HPHP_PDO_PGSQL_STATEMENT_H_

#include "hphp/runtime/ext/pdo/pdo_driver.h"
#include "pdo_pgsql_resource.h"
#include "pq.h"
#include "stdarg.h"

#define BOOLOID     16
#define BYTEAOID    17
#define INT8OID     20
#define INT2OID     21
#define INT4OID     23
#define TEXTOID     25
#define OIDOID      26

namespace HPHP {
    class PDOPgSqlStatement : public PDOStatement {
        friend PDOPgSqlConnection;

    public:
        DECLARE_RESOURCE_ALLOCATION(PDOPgSqlStatement);

        PDOPgSqlStatement(req::ptr<PDOPgSqlResource> conn, PQ::Connection* server);
        virtual ~PDOPgSqlStatement();

        bool create(const String& sql, const Array &options);

        virtual bool executer();
        virtual bool fetcher(PDOFetchOrientation ori, long offset);

        virtual bool describer(int colno);

        virtual bool getColumnMeta(int64_t colno, Array &return_value);
        virtual bool getColumn(int colno, Variant &value);

        virtual bool paramHook(PDOBoundParam* param, PDOParamEvent event_type);

        virtual bool cursorCloser();

        virtual bool support(SupportedMethod method);

    private:
        std::shared_ptr<PDOPgSqlConnection> m_conn;
        PQ::Connection* m_server;
        static unsigned long m_stmtNameCounter;
        static unsigned long m_cursorNameCounter;
        std::string m_stmtName;
        std::string m_resolvedQuery;
        std::string m_cursorName;
        std::string err_msg;
        PQ::Result m_result;
        bool m_isPrepared;
        bool m_hasParams;

        std::vector<Oid> param_types;
        std::vector<Variant> param_values;
        std::vector<int> param_lengths;
        std::vector<int> param_formats;

        std::vector<Oid> m_pgsql_column_types;

        long m_current_row;

        template <typename... T>
        std::string strprintf(const char* format, T... args){
            int size = snprintf(nullptr, 0, format, args...);
            std::string str(size, '\0');
            int actual_sz = snprintf(const_cast<char *>(str.c_str()), size + 1, format, args...);
            if (actual_sz != size) {
                throw std::exception();
            }
            return str;
        }

        std::string oriToStr(PDOFetchOrientation ori, long offset, bool& success){
            success = true;
            switch(ori){
                case PDO_FETCH_ORI_NEXT:
                    return std::string("NEXT");
                case PDO_FETCH_ORI_PRIOR:
                    return std::string("BACKWARD");
                case PDO_FETCH_ORI_FIRST:
                    return std::string("FIRST");
                case PDO_FETCH_ORI_LAST:
                    return std::string("LAST");
                case PDO_FETCH_ORI_ABS:
                    return strprintf("ABSOLUTE %ld", offset);
                case PDO_FETCH_ORI_REL:
                    return strprintf("RELATIVE %ld", offset);
                default:
                    success = false;
                    return std::string();
            }
        }
    };

}

#endif
