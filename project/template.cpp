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
#include <iomanip>   // IO Manipulations
#include <string>    // Strings
#include <fstream>   // File Input/Output
#include <cmath>     // Math Operations (pow)
#include <cstdlib>   // Extended Math Operations, String Conversion
#include <cctype>    // Char data type controls
#include <cstring>   // C-String Manipulations
#include <stdexcept> // Exception Classes
#include <cstddef>   // C Standard Definitions (NULL)
#include <sstream>   // String Streams
#include <vector>    // STL Vectors
#include <queue>     // STL Queue
#include <deque>     // STL Deque
#include <list>      // STL Linked List
#include <iterator>  // STL container Iterators
#include <algorithm> // STL algorithms
#include <map>       // STL Map associative container
#include <time.h>    // Time algorithms


using namespace std;

/*****************************************************************************
* Class Name: 
* Inherits: 
* Summary:       
*****************************************************************************/


template < class T >
/*****************************************************************************
*  template <class T>
*  void getNumber(T& number, string prompt, 
*                 string errMsg = "\nValue must be an integer!\n")
*
*  Desc: Error safe - read integer routine
*        On error, this function will re-prompt for the integer
*        until there is no input error.
*
*  Inputs:  prompt - string used to prompt for the integer
*           errMsg - error message you wish displayed if you get an error.
*  Outputs: number - number input from user
*****************************************************************************/
void getNumber(T& number, string prompt, 
               string errMsg = "\nValue must be an integer!\n")
{
   bool check = 1;
   while ( check ) // loop to retry input when invalid input entered
   {
      cout << prompt;
      cin >> number;
      if ( cin.fail() ) // test input stream for fail mode, and reset
      {
         cin.clear();
         cout << errMsg;
         cin.ignore( 80, '\n' );
         continue;
      }
      else
         check = 0;
   }
   cin.ignore( 80, '\n' );
   return;
}

/*****************************************************************************
* function main(): handle main control, call functions
* Opens a file, reads the data from the file, and passes the data to functions
*****************************************************************************/
int main(int argc, char* argv[])
{
   ifstream fin;
   bool test = false;
   if ( argc > 1 )
   {
      fin.open(argv[1]);
      if ( !fin.fail() )
         test = true;
      else
         cout << "\n\n\tInvalid file name! Try again.\n";
   }
   while (test != true)
   {
      cout << "Enter name of input file: ";
      string fileName;
      getline(cin, fileName);
      fin.open(fileName.c_str());
      if ( !fin.fail() )
         test = true;
      else
         cout << "\n\n\tInvalid file name! Try again.\n";
   }
   
   fin.close();
   
   return 0;
}

/*****************************************************************************
* (Function Name)
*
*  Desc: 
*
*  Inputs:  
*  Outputs: 
*  return:  
*****************************************************************************/
   