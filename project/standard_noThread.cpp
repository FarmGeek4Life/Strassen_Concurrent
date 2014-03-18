/***************************************************************************
* Program:
*    Exploration 3, Implement a Divide and Conquer Algorithm
*    Brother Neff, CS 306
* Author:
*    Bryson Gibbons
* Summary:
*    Implement Strassen's Algorithm for multiplying two matrices
*    
***************************************************************************/

#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
using namespace std;

// Stopping point for thread creation
int thread_Stop;

template <class T>
class Matrix
{
private:
   T** mRows;
   int mSize;

public:
   Matrix<T>(int size)
   {
      mRows = new T*[size];
      for (int i = 0; i < size; i++)
      {
         mRows[i] = new T[size];
      }   
      mSize = size;
   }

   Matrix<T>(const Matrix<T>& matrixB)
   {
      mSize = matrixB.getSize();

      mRows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         mRows[i] = new T[mSize];
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = matrixB.mRows[i][j];
         }
      }
   }

   Matrix<T>& operator=(const Matrix<T>& matrixB)
   {
      for (int i = 0; i < mSize; i++)
      {
         delete [] mRows[i];
      }
      delete [] mRows;

      mSize = matrixB.getSize();

      mRows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {  
         mRows[i] = new T[mSize];  
         for (int j = 0; j < mSize; ++j)  
            mRows[i][j] = matrixB.mRows[i][j];  
      }  
      return *this;  
   }  

   ~Matrix<T>()
   {
      for (int i = 0; i < mSize; i++)
      {
         delete [] mRows[i];
      }   
      delete [] mRows;
   }

   T* operator[](int row) const
   {
      return mRows[row];
   }

   int getSize() const
   {
      return mSize;
   }

   void read(istream& is) const
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            is >> mRows[i][j];
         }      
      }
   }

   void write(ostream& os) const
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            os << mRows[i][j] << " ";
         }
         os << endl;
      }
   }
   
   /**************************************************************************
    * Matrix multiplication management
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> mult(const Matrix<T> matrixB, Matrix<T>& result) const
   {
      for (int i = 0; i < mSize; ++i)
      {
         for (int j = 0; j < mSize; ++j )
         {
            this->multiply(i, j, matrixB, result);
         }
      }
      
      return result;
   }
   
   /************************************************************************
   * Low level matrix multiplier.....
   ***********************************************************************/
   void multiply(int i, int j, const Matrix<T>& matrixB, Matrix<T>& result) const
   {
      // Initialize to known value...
      result[i][j] = 0;
      for (int k = 0; k < mSize; ++k)
      {
         result[i][j] += (*this)[i][k] * matrixB[k][j];
      }
   }
   
   /************************************************************************
   * Low level matrix multiplier.....
   ***********************************************************************/
   void multiplyRow(int i, const Matrix<T>& matrixB, Matrix<T>& result) const
   {
      for (int j = 0; j < mSize; ++j)
      {
         // Initialize to known value...
         result[i][j] = 0;
         for (int k = 0; k < mSize; ++k)
         {
            result[i][j] += (*this)[i][k] * matrixB[k][j];
         }
      }
   }
};

template <class T>
istream& operator>>(istream& is, const Matrix<T>& m)
{
   m.read(is);
   return is;
}

template <class T>
ostream& operator<< (ostream& os, const Matrix<T>& m)
{
   m.write(os);
   return os;
}

int main(int argc, char* argv[])
{
   int size = 32;
   ifstream inFile;
   ifstream inFile2;
   string file;
   string file2;

   if (argc == 2)
   {
      file = argv[1];
      file2 = argv[1];
      thread_Stop = size / 4;
   }
   else if (argc == 3)
   {
      file = argv[1];
      file2 = argv[2];
      thread_Stop = size / 4;
   }
   else if (argc == 4)
   {
      file = argv[1];
      file2 = argv[2];
      size = atoi(argv[3]);
      thread_Stop = size / 4;
   }
   else if (argc > 4)
   {
      file = argv[1];
      file2 = argv[2];
      size = atoi(argv[3]);
      thread_Stop = atoi(argv[4]);
   }
   else 
   {
      cout << "Usage: " << argv[0] << " [file1] [file2] [size]\n";
   }
   if (thread_Stop > 200)
   {
      thread_Stop = 200;
   }

   Matrix<int> matrixA(size);
   Matrix<int> matrixB(size);

   inFile.open(file.c_str());
   
   if (inFile.is_open())
   {
      inFile >> matrixA;
      inFile.close();
   }
   else 
   {
      cerr << "Unable to open " + file;
      return 1;
   }

   inFile2.open(file2.c_str());
   
   if (inFile2.is_open())
   {
      inFile2 >> matrixB;
      inFile2.close();
   }
   else 
   {
      cerr << "Unable to open " + file2;
      return 1;
   }
   //cout << (matrixA * matrixB);
   Matrix<int> result(size);
   matrixA.mult(matrixB, result);
   cout << result;

   return 0;
}
