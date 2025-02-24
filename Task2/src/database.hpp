#pragma once
#include "connection.hpp"

using SqlResponse = std::vector<std::vector<std::string>>;

class Database
{
public:
    static bool TableExists(ConnectionPool& pool, const std::string& tableName)
    {
        const std::string sql = R"(
            SELECT EXISTS (
                SELECT 1
                FROM information_schema.tables 
                WHERE table_schema='public' AND table_name = $1
            )
        )";

        auto result = Execute(pool, sql, { tableName });
        return !result.empty() && result[0][0] == "t";
    }

    static bool EntryExists(ConnectionPool& pool, const std::string& tableName, int id)
    {
        const std::string sql = "SELECT EXISTS (SELECT 1 FROM " + tableName + " WHERE id = $1)";

        auto result = Execute(pool, sql, { std::to_string(id) });
        return !result.empty() && result[0][0] == "t";
    }

    static int CountEntries(ConnectionPool& pool, const std::string& tableName)
    {
        const std::string sql = "SELECT COUNT(*) FROM " + tableName + " WHERE deleted = false";

        SqlResponse response = Execute(pool, sql);
        if (!response.empty() && response.size() == 1 && response[0].size() == 1) {
            return std::stoi(response[0][0]);
        }

        return 0;
    }

    static SqlResponse GetEntries(ConnectionPool& pool, const std::string& tableName, bool showDeleted)
    {
        std::string sql = "SELECT * FROM " + tableName + (!showDeleted ? " WHERE deleted = false" : "");
        return Execute(pool, sql);
    }

    static SqlResponse Execute(ConnectionPool& pool, const std::string& sql, const pqxx::params& args = {})
    {
        SqlResponse response;

        try
        {
            auto conn = pool.getConnection();
            pqxx::work w(*conn);

            if (sql.find("SELECT") != std::string::npos || sql.find("RETURNING") != std::string::npos)
            {
                pqxx::result res = w.exec(sql, args);
                for (const auto& row : res)
                {
                    std::vector<std::string> rowData;
                    for (const auto& field : row)
                    {
                        rowData.push_back(field.c_str());
                    }
                    response.push_back(rowData);
                }
            }
            else
            {
                w.exec(sql, args);
            }

            w.commit();
            pool.pushConnection(conn);
        }
        catch (const pqxx::sql_error& e)
        {
            std::cerr << "SQL error: " << e.what() << "\n";
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << "\n";
        }

        return response;
    }

private:
    Database() = default;
    ~Database() = default;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
};