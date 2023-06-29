#include "server.hpp"

#include <thread>
#include <vector>

template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

using namespace std::literals;

using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

// Создаёт StringResponse с заданными параметрами
StringResponse MakeStringResponse(http::status status, std::string_view body, unsigned http_version,
								  bool keep_alive, std::string_view content_type = "text/html"sv) {
	StringResponse response(status, http_version);
	response.set(http::field::content_type, content_type);
	response.body() = body;
	response.content_length(body.size());
	response.keep_alive(keep_alive);
	return response;
}

StringResponse HandleRequest(StringRequest&& request) {
	const auto text_response = [&request](http::status status, std::string_view text) {
		return MakeStringResponse(status, text, request.version(), request.keep_alive());
	};
	return text_response(
	    http::status::ok,
	    "Hello, "s.append(request.target().substr(1))
	);

}

int main() {
	unsigned num_threads = 8;
    net::io_context ioc{static_cast<int>(num_threads)};

    const auto address = net::ip::make_address("0.0.0.0");
    constexpr net::ip::port_type port = 8080;
    http_server::ServeHttp(ioc, {address, port}, [](auto&& req, auto&& sender) {
        sender(HandleRequest(std::forward<decltype(req)>(req)));
    });

    RunWorkers(num_threads, [&ioc] {
        ioc.run();
    });
}