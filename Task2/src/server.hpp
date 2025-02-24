#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include "json.hpp"
#include "database.hpp"
#include "response.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class HttpServer
{
public:
    HttpServer(net::io_context& ioc, tcp::endpoint endpoint, const std::string& cnStr, size_t maxCn)
        : _acceptor(ioc, std::move(endpoint)),
        _pool(cnStr, maxCn)
    {
        doAccept();
    }

private:
    ConnectionPool _pool;
    tcp::acceptor _acceptor;

    void doAccept()
    {
        _acceptor.async_accept(beast::bind_front_handler(&HttpServer::onAccept, this));
    }

    void onAccept(beast::error_code ec, tcp::socket socket)
    {
        if (ec)
        {
            std::cerr << "Error on accepting: " << ec.message() << "\n";
        }
        else
        {
            std::thread(&HttpServer::handleRequest, this, std::move(socket)).detach();
        }
        doAccept();
    }

    void handleRequest(tcp::socket socket)
    {
        try
        {
            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(socket, buffer, req);

            if (req.method() == http::verb::options)
            {
                HttpResponse::handleOptions(req, socket);
                return;
            }

            if (req.method() == http::verb::post)
            {
                handlePost(req, socket);
            }
            else
            {
                HttpResponse::sendResponse(req, http::status::bad_request, "Only POST allowed", socket);
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error on handling request: " << e.what() << "\n";
        }
    }

    void handlePost(const http::request<http::string_body>& req, tcp::socket& socket)
    {
        try
        {
            auto jsonInput = nlohmann::json::parse(req.body());
            auto jsonResponse = processJson(jsonInput);
            HttpResponse::sendResponse(req, http::status::ok, jsonResponse.dump(), socket);
        }
        catch (const nlohmann::json::parse_error&)
        {
            HttpResponse::sendResponse(req, http::status::bad_request, "Invalid JSON", socket);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error in handlePost: " << e.what() << "\n";
            HttpResponse::sendResponse(req, http::status::internal_server_error, "Server error", socket);
        }
    }

    nlohmann::json processJson(const nlohmann::json& request)
    {
        nlohmann::json response;
        if (!request.contains("method"))
        {
            response["status"] = "error";
            response["message"] = "Missing method";
            return response;
        }

        const std::string method = request["method"];
        if (method == "checkScheme")
        {
            response["status"] = "success";
            response["studentsTableExists"] = Database::TableExists(_pool, "students");
        }
        else if (method == "initStudents")
        {
            return handleInitStudents(response);
        }
        else if (method == "countStudents")
        {
            response["status"] = "success";
            response["count"] = Database::CountEntries(_pool, "students");
        }
        else if (method == "createStudent")
        {
            return handleCreateStudent(request, response);
        }
        else if (method == "deleteStudent")
        {
            return handleDeleteStudent(request, response);
        }
        else if (method == "getStudents")
        {
            return handleGetStudents(request, response);
        }
        else
        {
            response["status"] = "error";
            response["message"] = "No such method";
        }
        return response;
    }

    nlohmann::json handleInitStudents(nlohmann::json& response)
    {
        if (Database::TableExists(_pool, "students"))
        {
            response["status"] = "error";
            response["message"] = "students table already exists";
        }
        else
        {
            std::string sql = R"(
                CREATE TABLE students (
                    id SERIAL PRIMARY KEY,
                    name TEXT NOT NULL,
                    surname TEXT NOT NULL,
                    middlename TEXT,
                    birthdate DATE,
                    groupname TEXT NOT NULL,
                    deleted BOOLEAN DEFAULT FALSE
                );)";
            Database::Execute(_pool, sql);
            response["status"] = "success";
        }
        return response;
    }

    nlohmann::json handleCreateStudent(const nlohmann::json& request, nlohmann::json& response)
    {
        std::string sql = "INSERT INTO students (name, surname, middlename, birthdate, groupname) VALUES($1, $2, $3, $4, $5) RETURNING id;";
        
        std::string name = request["name"];
        std::string surname = request["surname"];
        std::string middlename = request["middlename"];
        std::string birthdate = request["birthdate"];
        std::string group = request["group"];
        pqxx::params args = { name, surname, middlename, birthdate, group };

        response["status"] = "success";
        response["id"] = Database::Execute(_pool, sql, args)[0][0];
        return response;
    }

    nlohmann::json handleDeleteStudent(const nlohmann::json& request, nlohmann::json& response)
    {
        int id = request["id"];
        if (!Database::EntryExists(_pool, "students", id))
        {
            response["status"] = "error";
            response["idNotFound"] = true;
        }
        else
        {
            std::string sql = "UPDATE students SET deleted = true WHERE id = $1";
            Database::Execute(_pool, sql, { id });
            response["status"] = "success";
        }
        return response;
    }

    nlohmann::json handleGetStudents(const nlohmann::json& request, nlohmann::json& response)
    {
        auto result = Database::GetEntries(_pool, "students", request["showDeleted"]);
        response["status"] = "success";
        for (size_t i = 0; i < result.size(); i++)
        {
            response["data"][i]["id"] = result[i][0];
            response["data"][i]["name"] = result[i][1];
            response["data"][i]["surname"] = result[i][2];
            response["data"][i]["middlename"] = result[i][3];
            response["data"][i]["birthdate"] = result[i][4];
            response["data"][i]["group"] = result[i][5];
        }
        return response;
    }
};