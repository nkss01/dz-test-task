#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <pqxx/pqxx>
#include <boost/pool/pool.hpp>

class ConnectionPool
{
public:
    ConnectionPool(const std::string& _connectionStr, size_t _maxConnections)
            : _connectionStr(_connectionStr), _maxConnections(_maxConnections)
    {
        for (size_t i = 0; i < _maxConnections; i++)
        {
            _connections.push_back(std::make_shared<pqxx::connection>(_connectionStr));
        }
    }

    std::shared_ptr<pqxx::connection> getConnection()
    {
        if (_connections.empty())
        {
            throw std::runtime_error("No connections available");
        }

        auto cn = _connections.back();
        _connections.pop_back();
        return cn;
    }

    void pushConnection(std::shared_ptr<pqxx::connection> cn)
    {
        _connections.push_back(cn);
    }

private:
    std::vector<std::shared_ptr<pqxx::connection>> _connections;
    std::string _connectionStr;
    size_t _maxConnections;
};
