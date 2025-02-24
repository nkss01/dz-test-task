#pragma once
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include "json.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

class HttpResponse
{
public:
    static void sendResponse(const http::request<http::string_body>& req,
        http::status status,
        const std::string& body,
        tcp::socket& socket)
    {
        http::response<http::string_body> res(status, req.version());
        res.set(http::field::content_type, "application/json");
        res.set(http::field::access_control_allow_origin, "*");
        res.body() = body;
        res.prepare_payload();
        http::write(socket, res);
    }

    static void handleOptions(const http::request<http::string_body>& req, tcp::socket& socket)
    {
        http::response<http::string_body> res(http::status::ok, req.version());
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_methods, "POST, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type");
        res.set(http::field::access_control_expose_headers, "Content-Type");
        res.prepare_payload();
        http::write(socket, res);
    }
};