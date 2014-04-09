/*****************************************************************************
* Program:
*    Assignment __, (Program Name)
*    Brother Ercanbrack, CS235
* Author:
*    Bryson Gibbons
* Summary:
*    (Brief description of your program)
*****************************************************************************/

#include <iostream>  // Standard Header
#include <cmath>     // Math Operations (pow)
#include <cstdlib>   // Extended Math Operations, String Conversion

using namespace std;

/*****************************************************************************
* function main(): handle main control, call functions
* Opens a file, reads the data from the file, and passes the data to functions
*****************************************************************************/
int main(int argc, char** argv)
{
   bool useSeed = false;
   int seed = 0;
   unsigned int size = 0;
   if ( argc < 2 )
   {
      cout << "Usage: " << argv[0] << "[size] [(seed)]\n";
      return 1;
   }
   else
   {
      size = atoi(argv[1]);
      if ( argc >= 3 )
      {
         useSeed = true;
         seed = atoi(argv[2]);
         // Seed the pseudo-random sequence
         srand(seed);
      }
      else
      {
         srand((int)time(NULL));   // variable seed
      }
   }
   
   // using short, 1mill x 1mill: over 7GB of memory....
   // using char, 8192x8192 (2^13): 332 MB of hard drive space... 14 seconds to generate
   // using char, 16384x16384 (2^14): 1.3 GB of hard drive space... 55 seconds to generate
   
   // Using a seed, the output is consistent - it will be the same each time.
   
   // RAND_MAX is maximum size of signed integer.
   //char** matrix = new char*[size];
   //for (unsigned int i = 0; i < size; ++i)
   //{
   //   matrix[i] = new char[size];
   //   for (unsigned int j = 0; j < size; ++j)
   //   {
   //      matrix[i][j] = (char)(rand() % (int)(pow(2, 8)));
   //   }
   //}
   //
   //for (unsigned int i = 0; i < size; ++i)
   //{
   //   for (unsigned int j = 0; j < size; ++j)
   //   {
   //      cout << setw(5) << (int)matrix[i][j];
   //   }
   //   cout << endl;
   //   delete matrix[i];
   //}
   //delete matrix;
   // Less memory and time with no matrix allocation....
   for (unsigned int i = 0; i < size; ++i)
   {
      for (unsigned int j = 0; j < size; ++j)
      {
         // Cast the data to a char, and make sure it outputs as an int.
         cout << ((int)((char)(rand() % (int)(pow(2, 8))))) << " ";
      }
      cout << endl;
   }
   
   return 0;
}
