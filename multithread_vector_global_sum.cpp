#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <random>

constexpr size_t VECTOR_SIZE = 1000000;
const size_t MY_HW_CONC = std::thread::hardware_concurrency();
std::vector<uint8_t> v(VECTOR_SIZE);
uint64_t sum = 0;

void init() {
	for (std::vector<uint8_t>::iterator i = v.begin(); i != v.end(); ++i) {
		std::random_device device;
		std::mt19937 generator(device());
		std::uniform_int_distribution<int> distribution(0, std::numeric_limits<uint8_t>::max());
		
		*i = distribution(generator);
	}
}

void execute_thread(std::vector<uint8_t>::iterator begin, const std::vector<uint8_t>::iterator end) {
	while (begin != end) {
		sum += *begin++;
	}
}

int main(int argc, char *argv[]) {
	init();

	auto begin = std::chrono::system_clock::now();
	
	for (size_t i = 0; i < MY_HW_CONC - 1; ++i) {
		std::thread t(execute_thread, v.begin() + (VECTOR_SIZE / MY_HW_CONC) * i, v.begin() + (VECTOR_SIZE / MY_HW_CONC) * (i + 1));
		//t.join();
		t.detach();
	}
	std::thread t(execute_thread, v.begin() + (VECTOR_SIZE / MY_HW_CONC) * (MY_HW_CONC - 1), v.end());
	t.join(); //<- detach() makes some problems here


	auto end = std::chrono::system_clock::now() - begin;

	std::cout << "SUM= " << sum << "\nTIME: " << std::chrono::duration<double>(end).count() << std::endl;

	return 0;
}
