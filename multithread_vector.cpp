#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <random>

constexpr size_t VECTOR_SIZE = 1000000;
const size_t MY_HW_CONC = std::thread::hardware_concurrency();
std::vector<uint8_t> v(VECTOR_SIZE);

void init() {
  for (std::vector<uint8_t>::iterator i = v.begin(); i != v.end(); ++i) {
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(
        0, std::numeric_limits<uint8_t>::max());

    *i = distribution(generator);
  }
}

void execute_thread(std::vector<uint8_t>::iterator begin,
                    const std::vector<uint8_t>::iterator end,
                    std::promise<uint64_t> prom) {
  uint64_t sum = 0;

  while (begin != end) {
    sum += *begin++;
  }

  prom.set_value(sum);
}

int main(int argc, char *argv[]) {
  init();

  auto begin = std::chrono::system_clock::now();

  std::vector<std::promise<uint64_t>> prom(MY_HW_CONC);
  std::vector<std::future<uint64_t>> f(MY_HW_CONC);

  for (size_t i = 0; i < MY_HW_CONC; ++i)
    f[i] = prom[i].get_future();

  for (size_t i = 0; i < MY_HW_CONC - 1; ++i) {
    std::thread t(execute_thread, v.begin() + (VECTOR_SIZE / MY_HW_CONC) * i,
                  v.begin() + (VECTOR_SIZE / MY_HW_CONC) * (i + 1),
                  std::move(prom[i]));
    // t.join();
    t.detach();
  }
  std::thread t(execute_thread,
                v.begin() + (VECTOR_SIZE / MY_HW_CONC) * (MY_HW_CONC - 1),
                v.end(), std::move(prom.back()));
  // t.join();
  t.detach();

  auto end = std::chrono::system_clock::now() - begin;

  uint64_t sum = 0;
  for (size_t i = 0; i < MY_HW_CONC; ++i)
    sum += f[i].get();

  std::cout << "SUM= " << sum
            << "\nTIME: " << std::chrono::duration<double>(end).count()
            << std::endl;

  return 0;
}
