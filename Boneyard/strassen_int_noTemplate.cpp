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
#include <thread>
using namespace std;

// Stopping point for thread creation
int thread_Stop;

class Matrix
{
private:
   int** mRows;
   int mSize;

public:
   Matrix(int size)
   {
      mRows = new int*[size];
      for (int i = 0; i < size; i++)
      {
         mRows[i] = new int[size];
      }   
      mSize = size;
   }

   Matrix(const Matrix& matrixB)
   {
      mSize = matrixB.getSize();

      mRows = new int*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         mRows[i] = new int[mSize];
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = matrixB.mRows[i][j];
         }
      }
   }

   Matrix& operator=(const Matrix& matrixB)
   {
      for (int i = 0; i < mSize; i++)
      {
         delete [] mRows[i];
      }
      delete [] mRows;

      mSize = matrixB.getSize();

      mRows = new int*[mSize];
      for (int i = 0; i < mSize; i++)
      {  
         mRows[i] = new int[mSize];  
         for (int j = 0; j < mSize; ++j)  
            mRows[i][j] = matrixB.mRows[i][j];  
      }  
      return *this;  
   }  

   ~Matrix()
   {
      for (int i = 0; i < mSize; i++)
      {
         delete [] mRows[i];
      }   
      delete [] mRows;
   }

   int* operator[](int row) const
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

   Matrix operator+(Matrix matrixB)
   {
      Matrix result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            result[i][j] = (*this)[i][j] + matrixB[i][j];
         }
      }
      return result;
   }

   Matrix operator-(Matrix matrixB)
   {
      Matrix result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            result[i][j] = (*this)[i][j] - matrixB[i][j];
         }
      }
      return result;
   }
   
   /**************************************************************************
    * Constructor to build a matrix from quadrants
    *************************************************************************/
   Matrix(const Matrix& copy00, const Matrix& copy01, const Matrix& copy10, const Matrix& copy11)
   {
      // Assume that all parameters matrices are equal in size
      mSize = copy00.getSize() * 2;
      
      mRows = new int*[mSize];
      // Calculate half of the size to save iterations and operations
      int h = mSize / 2;
      for (int i = 0; i < h; i++)
      {
         // Allocate memory for two rows each time
         // Start at the top row of each half
         mRows[i] = new int[mSize];
         mRows[i + h] = new int[mSize];
         for (int j = 0; j < h; j++)
         {
            // Use i, j, and h to copy 1 value from each quadrant to 
            //    the correct position with each iteration
            mRows[i][j] = copy00.mRows[i][j];
            mRows[i][j + h] = copy01.mRows[i][j];
            mRows[i + h][j] = copy10.mRows[i][j];
            mRows[i + h][j + h] = copy11.mRows[i][j];
         }
      }
   }
   
   /**************************************************************************
    * Get the specified quadrant of the Matrix
    *************************************************************************/
   Matrix getQuadrant(int row, int col) const
   {
      Matrix result(mSize / 2);
      int rRow = 0;
      int rCol = 0;
      // Calculate the quadrant index limits based off of the input row and col
      int qRowMin = row * (mSize / 2);
      int qRowMax = ((row + 1) * (mSize / 2));
      int qColMin = col * (mSize / 2);
      int qColMax = ((col + 1) * (mSize / 2));
      
      // Copy data from the row, col quadrant to result.
      for (int qRow = qRowMin; qRow < qRowMax; ++qRow, ++rRow)
      {
         // Make sure to reset rCol for each row.
         rCol = 0;
         for (int qCol = qColMin; qCol < qColMax; ++qCol, ++rCol)
         {
            result[rRow][rCol] = mRows[qRow][qCol];
         }
      }
      return result;
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix mult(const Matrix matrixB, Matrix& result) const
   {
      //Matrix result(mSize);
      if (mSize > thread_Stop)
      {
         // Temporary Matrices to hold the 7 multiplication results
         Matrix m1(mSize / 2);
         Matrix m2(mSize / 2);
         Matrix m3(mSize / 2);
         Matrix m4(mSize / 2);
         Matrix m5(mSize / 2);
         Matrix m6(mSize / 2);
         Matrix m7(mSize / 2);
         
         // Four quadrants for each matrix being multiplied
         Matrix a00(getQuadrant(0, 0));
         Matrix a01(getQuadrant(0, 1));
         Matrix a10(getQuadrant(1, 0));
         Matrix a11(getQuadrant(1, 1));
         Matrix b00(matrixB.getQuadrant(0, 0));
         Matrix b01(matrixB.getQuadrant(0, 1));
         Matrix b10(matrixB.getQuadrant(1, 0));
         Matrix b11(matrixB.getQuadrant(1, 1));
         
         // Get the 7 multiplication results
         // m1 = (a00 + a11) * (b00 + b11);
         // m2 = (a10 + a11) *  b00;
         // m3 =  a00 *        (b01 - b11);
         // m4 =  a11 *        (b10 - b00);
         // m5 = (a00 + a01) *  b11;
         // m6 = (a10 - a00) * (b00 + b01);
         // m7 = (a01 - a11) * (b10 + b11);
         thread t1 = thread(&Matrix::mult, (a00 + a11), (b00 + b11), std::ref(m1));
         thread t2 = thread(&Matrix::mult, (a10 + a11), (b00)      , std::ref(m2));
         thread t3 = thread(&Matrix::mult,  a00       , (b01 - b11), std::ref(m3));
         thread t4 = thread(&Matrix::mult,  a11       , (b10 - b00), std::ref(m4));
         thread t5 = thread(&Matrix::mult, (a00 + a01), (b11)      , std::ref(m5));
         thread t6 = thread(&Matrix::mult, (a10 - a00), (b00 + b01), std::ref(m6));
         thread t7 = thread(&Matrix::mult, (a01 - a11), (b10 + b11), std::ref(m7));

         t1.join();
         t2.join();
         t3.join();
         t4.join();
         t5.join();
         t6.join();
         t7.join();
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         a00 = m1 + m4 - m5 + m7;
         a01 = m3 + m5;
         a10 = m2 + m4;
         a11 = m1 + m3 - m2 + m6;
         
         // Reassemble the quadrants into a single whole
         result = Matrix(a00, a01, a10, a11);
      }
      else if (mSize > 1)
      {
         // Temporary Matrices to hold the 7 multiplication results
         Matrix m1(mSize / 2);
         Matrix m2(mSize / 2);
         Matrix m3(mSize / 2);
         Matrix m4(mSize / 2);
         Matrix m5(mSize / 2);
         Matrix m6(mSize / 2);
         Matrix m7(mSize / 2);
         
         // Four quadrants for each matrix being multiplied
         Matrix a00(getQuadrant(0, 0));
         Matrix a01(getQuadrant(0, 1));
         Matrix a10(getQuadrant(1, 0));
         Matrix a11(getQuadrant(1, 1));
         Matrix b00(matrixB.getQuadrant(0, 0));
         Matrix b01(matrixB.getQuadrant(0, 1));
         Matrix b10(matrixB.getQuadrant(1, 0));
         Matrix b11(matrixB.getQuadrant(1, 1));
         
         // Get the 7 multiplication results
         //m1 = (a00 + a11) * (b00 + b11);
         //m2 = (a10 + a11) *  b00;
         //m3 =  a00 *        (b01 - b11);
         //m4 =  a11 *        (b10 - b00);
         //m5 = (a00 + a01) *  b11;
         //m6 = (a10 - a00) * (b00 + b01);
         //m7 = (a01 - a11) * (b10 + b11);
         
         (a00 + a11).mult (b00 + b11, m1);
         (a10 + a11).mult (b00      , m2);
          a00       .mult (b01 - b11, m3);
          a11       .mult (b10 - b00, m4);
         (a00 + a01).mult (b11      , m5);
         (a10 - a00).mult (b00 + b01, m6);
         (a01 - a11).mult (b10 + b11, m7);
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         a00 = m1 + m4 - m5 + m7;
         a01 = m3 + m5;
         a10 = m2 + m4;
         a11 = m1 + m3 - m2 + m6;
         
         // Reassemble the quadrants into a single whole
         result = Matrix(a00, a01, a10, a11);
      }
      else
      {
         // Assume a matrix of size 1
         result[0][0] = mRows[0][0] * matrixB[0][0];
      }
      return result;
   }
};

istream& operator>>(istream& is, const Matrix& m)
{
   m.read(is);
   return is;
}

ostream& operator<< (ostream& os, const Matrix& m)
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

   Matrix matrixA(size);
   Matrix matrixB(size);

   inFile.open(file.c_str());
   
   if (inFile.is_open())
   {
      inFile >> matrixA;
      inFile.close();
   }
   else 
   {
      cout << "Unable to open " + file;
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
      cout << "Unable to open " + file2;
      return 1;
   }
   //cout << (matrixA * matrixB);
   Matrix result(size);
   matrixA.mult(matrixB, result);
   cout << result;

   return 0;
}
