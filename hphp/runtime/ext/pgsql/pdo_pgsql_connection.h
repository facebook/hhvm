#ifndef incl_HPHP_PDO_PGSQL_CONNECTION_H_
#define incl_HPHP_PDO_PGSQL_CONNECTION_H_

#include "hphp/runtime/ext/pdo/pdo_driver.h"
#include "pq.h"

#define PHP_PDO_PGSQL_CONNECTION_FAILURE_SQLSTATE "08006"
namespace HPHP {
    class PDOPgSqlStatement;

    class PDOPgSqlConnection : public PDOConnection {
        friend class PDOPgSqlStatement;
    public:
        PDOPgSqlConnection();
        virtual ~PDOPgSqlConnection();

        virtual bool create(const Array &options);

        virtual int64_t doer(const String& sql);

        virtual bool closer();

        virtual bool begin();
        virtual bool commit();
        virtual bool rollback();

        virtual bool checkLiveness();

        virtual bool quoter(const String& input, String &quoted, PDOParamType paramtype);

        virtual bool support(SupportedMethod method);

        virtual String lastId(const char *name);

        virtual int getAttribute(int64_t attr, Variant &value);
        virtual bool setAttribute(int64_t attr, const Variant &value);

        virtual bool fetchErr(PDOStatement *stmt, Array &info);

        String pgsqlLOBCreate();

        bool preparer(const String& sql, sp_PDOStatement *stmt, const Variant& options) override;

    private:
        PQ::Connection* m_server;
        Oid pgoid;
        ExecStatusType m_lastExec;
        std::string err_msg;
        bool m_emulate_prepare;
        const char* sqlstate(PQ::Result& result);
        void handleError(PDOPgSqlStatement* stmt, const char* sqlState, const char* msg);
        bool transactionCommand(const char* command);
        void testConnection();

    };
}

#endif
