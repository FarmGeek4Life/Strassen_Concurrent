
#define _GLIBCXX_USE_NANOSLEEP

#include <iostream>
#include <thread>
//#include <unistd.h>
//#include <chrono>
using namespace std;

void myThread(int x)
{
   for (int i = 0; i < 4; ++i)
   {
      cout << "I sleep for " << x << " seconds!\n";
      // sleep(x);
      this_thread::sleep_for(chrono::seconds(x));
   }
}

int main(int argc, char** argv)
{
   cout << "I ran!!!\n";
   thread first(myThread,2);
   thread second(myThread,3);
   thread third(myThread,4);
   
   first.join();
   second.join();
   third.join();
   return 0;
}
