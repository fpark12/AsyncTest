#include <iostream>
#include <string>
#include <sstream>
#include <future>
#include <thread>
#include <memory>
#include <asio.hpp>
//#include <asio/spawn.hpp>


using asio::ip::tcp;

/*! HTTP Client object. */
/*
class Request
{
	asio::io_service io_service_;
	std::promise<std::string> result_;

public:

	auto Fetch(const std::string& host) {

		asio::spawn(io_service_, std::bind(&Request::Fetch_, this,
			host, std::placeholders::_1));
		std::thread([=]() { io_service_.run();}).detach();

		return result_.get_future();
	}

private:

	void Fetch_(const std::string& host, asio::yield_context yield) {
		std::string rval;
		asio::error_code ec;

		try {
			tcp::resolver resolver(io_service_);

			decltype(address_it) addr_end;

			for(; address_it != addr_end; ++address_it) {
				// Construct a TCP socket instance
				tcp::socket sck(io_service_);

				sck.async_connect(*address_it, yield[ec]);
				if (ec) {
					std::cerr << "Failed to connect to "
						<< address_it->endpoint() << std::endl;

					// Try another IP
					continue;
				}

				asio::async_write(sck,
					asio::buffer(GetRequest(host)),
					yield);

				char reply[1024] {}; // Zero-initialize the buffer

									 // Async read data until we fail. (As in the other examples)
				while(!ec) {
					const auto rlen = sck.async_read_some(
						asio::mutable_buffers_1(reply, sizeof(reply)),
						yield[ec]);

					// Append the read data to the data we will return
					rval.append(reply, rlen);
				}

				result_.set_value(move(rval));
				return; // Success!
			}

			// We failed.
			throw std::runtime_error("Unable to connect to any host");
		} catch(...) {

			result_.set_exception(std::current_exception());
			return;
		}
	}

	// Construct a simple HTTP request to the host
	std::string GetRequest(const std::string& host) const {
		std::ostringstream req;
		req << "GET / HTTP/1.1\r\nHost: " << host << " \r\n"
			<< "Connection: close\r\n\r\n";

		return req.str();
	}
};

int main(int argc, char *argv[])
{
	// Check that we have one and only one argument (the host-name)
	assert(argc == 2 && *argv[1]);

	// Construct our HTTP Client object
	Request req;

	// Initiate the fetch and get the future
	auto result = req.Fetch(argv[1]);

	// Wait for the other thread to do it's job
	result.wait();

	try {
		// Get the page or an exception
		std::cout << result.get();
	} catch(const std::exception& ex) {
		// Explain to the user that there was a problem
		std::cerr << "Caught exception " << ex.what() << std::endl;

		// Error exit
		return -1;
	} catch(...) {
		// Explain to the user that there was an ever bigger problem
		std::cerr << "Caught exception!" << std::endl;

		// Error exit
		return -2;
	}

	// Successful exit
	return 0;
}
#include <type_traits>
#include <cstdio>

template <typename T>
struct A_impl
{
	using type = std::false_type;
};

template <typename T>
using A = typename A_impl<T>::type;

template <bool... B>
struct logic_helper;

template <bool... B>
using none_t = std::is_same<logic_helper<B...>, logic_helper<(B && false)...>>;

template <typename... C>
struct Foo
{
	// Compile error
	//using FooType = none_t<A<C>::value...>;

	// Work around
	using FooType2 = logic_helper<A<C>::value...>;
	using FooType3 = logic_helper<(A<C>::value && false)...>;
	using FooType4 = std::is_same<FooType2, FooType3>;
};
*/