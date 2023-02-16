#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main() {
    try {
        // Create an io_context object to handle asynchronous I/O operations
        boost::asio::io_context io_context;

        // Resolve the hostname and service name to a list of endpoints
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("www.example.com", "http");

        // Establish a connection to the first endpoint in the list
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Send an HTTP GET request
        std::ostringstream request_stream;
        request_stream << "GET / HTTP/1.1\r\n";
        request_stream << "Host: www.example.com\r\n";
        request_stream << "Connection: close\r\n\r\n";
        std::string request = request_stream.str();
        boost::asio::write(socket, boost::asio::buffer(request));

        // Read the response from the server
        boost::asio::streambuf response_buffer;
        boost::asio::read_until(socket, response_buffer, "\r\n");

        // Check that the response status line is "HTTP/1.1 200 OK"
        std::istream response_stream(&response_buffer);
        std::string http_version, status_code, status_message;
        response_stream >> http_version >> status_code >> status_message;
        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            throw std::runtime_error("Invalid response");
        }
        if (status_code != "200") {
            throw std::runtime_error("Response returned with status code " + status_code);
        }

        // Read the response headers, which are terminated by a blank line
        boost::asio::read_until(socket, response_buffer, "\r\n\r\n");

        // Output the response headers
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
            std::cout << header << "\n";
        }
        std::cout << "\n";

        // Read the remaining response body
        if (response_buffer.size() > 0) {
            std::cout << boost::asio::buffer_cast<const char*>(response_buffer.data()) << std::endl;
        }
        boost::system::error_code ec;
        while (boost::asio::read(socket, response_buffer,
            boost::asio::transfer_at_least(1), ec)) {
            std::cout << boost::asio::buffer_cast<const char*>(response_buffer.data()) << std::endl;
        }
        if (ec != boost::asio::error::eof) {
            throw boost::system::system_error(ec);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
