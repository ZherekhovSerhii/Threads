#include <future>
#include <iostream>
#include <thread>
#include <vector>

#include <ctime>

constexpr size_t MY_HW_CONC = 4;
constexpr size_t VECTOR_SIZE = 1000000;

using vector_ptr = std::_Vector_iterator<
    class std::_Vector_val<struct std::_Simple_types<unsigned char>>>;

void init(std::vector<uint8_t> &v) {
  srand(static_cast<unsigned int>(time(0)));
  for (vector_ptr i = v.begin(); i < v.end(); ++i) {
    *i = rand() % (std::numeric_limits<uint8_t>::max() + 1);
  }
}

void execute_thread(std::vector<uint8_t> &v, vector_ptr begin, vector_ptr end,
                    std::promise<uint64_t> prom) {
  uint64_t sum = 0;

  for (vector_ptr i = begin; i != end; ++i) {
    // std::cout << "!" << std::endl;
    sum += *i;
  }

  prom.set_value(sum);
  // std::cout << "Exit thread" << std::endl;
}

int main(int argc, char *argv[]) {
  std::vector<uint8_t> v(VECTOR_SIZE);
  init(v);

  unsigned int start_time = clock();

  std::promise<uint64_t> prom[4];
  std::future<uint64_t> f[4];
  for (size_t i = 0; i < 4; ++i) {
    f[i] = prom[i].get_future();
  }

  for (size_t i = 0; i < MY_HW_CONC; ++i) {
    std::thread t(
        execute_thread, std::ref(v), v.begin() + (VECTOR_SIZE / MY_HW_CONC) * i,
        v.begin() + (VECTOR_SIZE / MY_HW_CONC) * (i + 1), std::move(prom[i]));
    // t.join();
    t.detach();
  }

  uint64_t sum = 0;
  for (size_t i = 0; i < 4; ++i) {
    sum += f[i].get();
  }

  unsigned int end_time = clock();
  unsigned int search_time = end_time - start_time;

  std::cout << "SUM= " << sum << "\nTIME: " << search_time << std::endl;
  return 0;
}
