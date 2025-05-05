#include "megatron.hpp"

int main() {
  Megatron *megatron = new Megatron();
  megatron->init();
  delete megatron;
  return 0;
}
