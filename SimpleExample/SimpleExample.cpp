#include <iostream>
#include <chrono>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

std::vector<char> vBuffer(1 * 1024);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length)
		{
			if (!ec)
			{
				std::cout << "\n\nRead " << length << " bytes\n\n";

				for (int i = 0; i < length; i++)
				{
					std::cout << vBuffer[i];
				}

				GrabSomeData(socket);
			}
		}
	);
}

int main()
{
	asio::error_code ec;

	//Create a context platform specific interface
	asio::io_context context;

	//Give asio some fake tasks so that the context doesnt fiinish
	asio::io_context::work idleWork(context);

	//start context on its own thread
	std::thread thrContext = std::thread([&]() {context.run(); });

	//Get the address of somewhere we wish to connect
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

	//Create a socket, the context will deilver the implementation
	asio::ip::tcp::socket socket(context);

	//Try to connect via socket
	socket.connect(endpoint, ec);

	if (!ec)
	{
		std::cout << "Connected!" << std::endl;
	}
	else
	{
		std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;

	}

	if (socket.is_open()) 
	{
		GrabSomeData(socket);

		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: david-barr.co.uk\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

		//Delaying main thread while asio handles data transfer in the background
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20000ms);

		context.stop();
		if (thrContext.joinable()) thrContext.join();

	}

	system("pause");

	return 0;
}