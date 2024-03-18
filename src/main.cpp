#include "ServerBase.h"
# include <iostream>
int main() {
  std::cout<<" hello myserver"<<std::endl;
  ServerBase server(8033);
  // wait for client connection
  // wait some time
  std::this_thread::sleep_for(std::chrono::seconds(100));
  return 0;
}