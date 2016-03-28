#include "main_app.h"

int main(int argc, char* argv[])
{
  gr::main_app app(argc, argv);
  int c = app.execute();
  std::cout << "exit_code " << c << std::endl;
  return c;
  //return app.execute();
}
