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
#include <mutex>
#include <unistd.h>
using namespace std;

template <class T>
class Matrix
{
private:
   T** mRows;
   int mSize;
   bool colAlloc;
   bool rowAlloc;
   bool started;
   bool startWrong;
   bool joined;

public:
   Matrix<T>(int size, bool alloc)
   {
      if (alloc)
      {
         mRows = new T*[size];
         for (int i = 0; i < size; i++)
         {
            mRows[i] = new T[size];
         }
         colAlloc = true;
         rowAlloc = true;
      }
      else
      {
         colAlloc = false;
         rowAlloc = false;
      }
      mSize = size;
      started = false;
      startWrong = false;
      joined = false;
   }
   
   Matrix<T>(int size)
   {
      mRows = new T*[size];
      for (int i = 0; i < size; i++)
      {
         mRows[i] = new T[size];
      }   
      mSize = size;
      colAlloc = true;
      rowAlloc = true;
      started = false;
      startWrong = false;
      joined = false;
   }

   Matrix<T>(const Matrix<T>& matrixB)
   {
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      
      mRows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         mRows[i] = new T[mSize];
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = matrixB.mRows[i][j];
         }
      }
      started = false;
      startWrong = false;
      joined = false;
   }

   Matrix<T> steal(Matrix<T>& matrixB)
   {
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      
      // Steal the pointers, instead of copying data...
      mRows = matrixB.mRows;
      matrixB.colAlloc = false;
      matrixB.rowAlloc = false;
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   Matrix<T>(const Matrix<T>& matrixA, const Matrix<T>& matrixB, bool add)
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      
      mRows = new T*[mSize];
      if (add)
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] + matrixB.mRows[i][j];
            }
         }
      }
      else
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] - matrixB.mRows[i][j];
            }
         }
      }
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   * Wrapper to use with threading
   *********************************************************************/
   void addFill(const Matrix<T>& matrixB, Matrix<T>*& newMatrix, bool add) const
   {
      newMatrix = new Matrix<T>(*this, matrixB, add);
   }
   
   /*********************************************************************
   * Copy Matrix
   * Wrapper to use with threading
   *********************************************************************/
   void copyTo(Matrix<T>*& newMatrix) const
   {
      newMatrix = new Matrix<T>(*this);
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void allocMath(const Matrix<T>& matrixA, const Matrix<T>& matrixB, bool add)
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      
      mRows = new T*[mSize];
      if (add)
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] + matrixB.mRows[i][j];
            }
         }
      }
      else
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] - matrixB.mRows[i][j];
            }
         }
      }
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void allocCopy(const Matrix<T>& matrixB)
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;

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
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void allocZero()
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      colAlloc = true;
      rowAlloc = true;

      mRows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         mRows[i] = new T[mSize];
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = 0;
         }
      }
   }

   Matrix<T>& operator=(const Matrix<T>& matrixB)
   {
      if (this == &matrixB)
      {
         return *this;
      }
      if (mSize != matrixB.getSize())
      {
         if (colAlloc && rowAlloc)
         {
            for (int i = 0; i < mSize; i++)
            {
               delete [] mRows[i];
            }
            delete [] mRows;
         }
         else if (colAlloc && !rowAlloc)
         {
            delete [] mRows;
         }
         
         mSize = matrixB.getSize();
         
         mRows = new T*[mSize];
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; ++j)
               mRows[i][j] = matrixB.mRows[i][j];
         }
         colAlloc = true;
         rowAlloc = true;
      }
      else
      {
         for (int i = 0; i < mSize; i++)
         {
            for (int j = 0; j < mSize; ++j)
               mRows[i][j] = matrixB.mRows[i][j];
         }
      }
      return *this;
   }  
   
   /**************************************************************************
    * Destructor: Conditional on whether memory is allocated by the calling object
    *************************************************************************/
   ~Matrix<T>()
   {
      // Only delete the memory if this object allocated it
      if (colAlloc && rowAlloc)
      {
         for (int i = 0; i < mSize; i++)
         {
            delete [] mRows[i];
         }
         delete [] mRows;
      }
      else if (colAlloc && !rowAlloc)
      {
         delete [] mRows;
      }
   }
   
   /**************************************************************************
    * Erase the memory, used to reduce the amount of memory used
    *************************************************************************/
   void erase()
   {
      // Only delete the memory if this object allocated it
      if (colAlloc && rowAlloc)
      {
         for (int i = 0; i < mSize; i++)
         {
            delete [] mRows[i];
         }
         delete [] mRows;
      }
      else if (colAlloc && !rowAlloc)
      {
         delete [] mRows;
      }
      colAlloc = false;
      rowAlloc = false;
   }
   
   /**************************************************************************
    * Reallocate erased memory, used to reduce the amount of memory used
    *************************************************************************/
   void reallocate()
   {
      if (!colAlloc && !rowAlloc)
      {
         mRows = new T*[mSize];
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
         }
      }
      else if (!colAlloc && rowAlloc)
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
         }
      }
      colAlloc = true;
      rowAlloc = true;
   }

   T* operator[](int row) const
   {
      return mRows[row];
   }

   int getSize() const
   {
      return mSize;
   }

   void read(std::istream& is) const
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            is >> mRows[i][j];
         }      
      }
   }

   void write(std::ostream& os) const
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            os << mRows[i][j] << " ";
         }
         os << std::endl;
      }
   }

   Matrix<T> operator+(Matrix<T> matrixB)
   {
      Matrix<T> result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            result[i][j] = (*this)[i][j] + matrixB[i][j];
         }
      }
      return result;
   }

   Matrix<T> operator-(Matrix<T> matrixB)
   {
      Matrix<T> result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            result[i][j] = (*this)[i][j] - matrixB[i][j];
         }
      }
      return result;
   }

   Matrix<T>& operator+=(Matrix<T> matrixB)
   {
      Matrix<T> result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] += matrixB[i][j];
         }
      }
      return *this;
   }

   Matrix<T>& operator-=(Matrix<T> matrixB)
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] -= matrixB[i][j];
         }
      }
      return *this;
   }
   
   Matrix<T>& op00_11(const Matrix<T>& matrixA, const Matrix<T>& matrixB, const Matrix<T>& matrixC, const Matrix<T>& matrixD)
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j] - matrixC[i][j] + matrixD[i][j];
         }
      }
      return *this;
   }
   
   Matrix<T>& op01_10(const Matrix<T>& matrixA, const Matrix<T>& matrixB)
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j];
         }
      }
      return *this;
   }
   
   std::mutex mMutex;
   std::mutex mCVMutex;
   std::mutex mWMutex;
   
   /********************************************************************************************
   * Opportunistic math: as soon as the necessary threads complete, do the math
   ********************************************************************************************/
   //Matrix<T>& op00_11(const Matrix<T>& matrixA, const Matrix<T>& matrixB, const Matrix<T>& matrixC, const Matrix<T>& matrixD)
   //Matrix<T>& op00_11_con(const Matrix<T>& matrixA, const Matrix<T>& matrixB, const Matrix<T>& matrixC, const Matrix<T>& matrixD, std::thread* tA, std::thread* tB, std::thread* tC, std::thread* tD)
   Matrix<T>& op00_11_con(Matrix<T>& matrixA, Matrix<T>& matrixB, Matrix<T>& matrixC, Matrix<T>& matrixD, std::thread t[], int tA, int tB, int tC, int tD)
   {
      //std::chrono::milliseconds wait = std::chrono::milliseconds(1000);
      //{
      //std::unique_lock<std::mutex> lkA(matrixA.mCVMutex);
      //std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
      //matrixA.mCV.wait_until(lkA, now + wait);
      //}
      {
      std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      if (!matrixA.started && !matrixA.startWrong)
      {
         //cerr << "Bad mutex grab A!!!" << endl;
         matrixA.started = true;
         matrixA.startWrong = true;
         matrixA.mMutex.unlock();
      }
      else if (matrixA.startWrong)
      {
         //cerr << "Bad mutex grab looping A!!!" << endl;
         matrixA.mMutex.unlock();
         while (!matrixA.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tA].joinable())
      {
      //std::cerr << "Passed wait: " << tA << "\n";
         try
         {
            t[tA].join();
            matrixA.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      if (!matrixB.started && !matrixB.startWrong)
      {
         //cerr << "Bad mutex grab B!!!" << endl;
         matrixB.started = true;
         matrixB.startWrong = true;
         matrixB.mMutex.unlock();
      }
      else if (matrixB.startWrong)
      {
         //cerr << "Bad mutex grab looping B!!!" << endl;
         matrixB.mMutex.unlock();
         while (!matrixB.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tB].joinable())
      {
      //std::cerr << "Passed wait: " << tB << "\n";
         try
         {
            t[tB].join();
            matrixB.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkC(matrixC.mMutex);
      if (!matrixC.started && !matrixC.startWrong)
      {
         //cerr << "Bad mutex grab C!!!" << endl;
         matrixC.started = true;
         matrixC.startWrong = true;
         matrixC.mMutex.unlock();
      }
      else if (matrixC.startWrong)
      {
         //cerr << "Bad mutex grab looping C!!!" << endl;
         matrixC.mMutex.unlock();
         while (!matrixC.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tC].joinable())
      {
      //std::cerr << "Passed wait: " << tC << "\n";
         try
         {
            t[tC].join();
            matrixC.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkD(matrixD.mMutex);
      if (!matrixD.started && !matrixD.startWrong)
      {
         //cerr << "Bad mutex grab D!!!" << endl;
         matrixD.started = true;
         matrixD.startWrong = true;
         matrixD.mMutex.unlock();
      }
      else if (matrixD.startWrong)
      {
         //cerr << "Bad mutex grab looping D!!!" << endl;
         matrixD.mMutex.unlock();
         while (!matrixD.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tD].joinable())
      {
      //std::cerr << "Passed wait: " << tD << "\n";
         try
         {
            t[tD].join();
            matrixD.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j] - matrixC[i][j] + matrixD[i][j];
         }
      }
      return *this;
   }
   
   /********************************************************************************************
   * Opportunistic math: as soon as the necessary threads complete, do the math
   ********************************************************************************************/
   //Matrix<T>& op01_10(const Matrix<T>& matrixA, const Matrix<T>& matrixB)
   Matrix<T>& op01_10_con(Matrix<T>& matrixA, Matrix<T>& matrixB, std::thread t[], int tA, int tB)
   {
      //std::chrono::milliseconds wait = std::chrono::milliseconds(1000);
      //{
      //std::unique_lock<std::mutex> lkA(matrixA.mCVMutex);
      //std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
      //matrixA.mCV.wait_until(lkA, now + wait);
      //}
      {
      std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      if (!matrixA.started && !matrixA.startWrong)
      {
         //cerr << "Bad mutex grab A!!!" << endl;
         matrixA.started = true;
         matrixA.startWrong = true;
         matrixA.mMutex.unlock();
      }
      else if (matrixA.startWrong)
      {
         //cerr << "Bad mutex grab looping A!!!" << endl;
         matrixA.mMutex.unlock();
         while (!matrixA.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tA].joinable())
      {
      //std::cerr << "Passed wait: " << tA << "\n";
         try
         {
            t[tA].join();
            matrixA.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      if (!matrixB.started && !matrixB.startWrong)
      {
         cerr << "Bad mutex grab B!!!" << endl;
         matrixB.started = true;
         matrixB.startWrong = true;
         matrixB.mMutex.unlock();
      }
      else if (matrixB.startWrong)
      {
         cerr << "Bad mutex grab looping B!!!" << endl;
         matrixB.mMutex.unlock();
         while (!matrixB.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tB].joinable())
      {
      //std::cerr << "Passed wait: " << tB << "\n";
         try
         {
            t[tB].join();
            matrixB.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j];
         }
      }
      return *this;
   }
   
   /**************************************************************************
    * Constructor to build a matrix from quadrants
    *************************************************************************/
   Matrix<T>(const Matrix<T>& copy00, const Matrix<T>& copy01, const Matrix<T>& copy10, const Matrix<T>& copy11)
   {
      // Assume that all parameters matrices are equal in size
      mSize = copy00.mSize * 2;
      
      mRows = new T*[mSize];
      // Calculate half of the size to save iterations and operations
      int h = mSize / 2;
      for (int i = 0; i < h; i++)
      {
         // Allocate memory for two rows each time
         // Start at the top row of each half
         mRows[i] = new T[mSize];
         mRows[i + h] = new T[mSize];
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
      colAlloc = true;
      rowAlloc = true;
   }
   
   /**************************************************************************
    * Get the specified quadrant of the Matrix
    * This uses the pointers to the values in the original matrix;
    * Editing this matrix will also edit the specific quadrant of the original
    *************************************************************************/
   Matrix<T>(const Matrix<T>& original, bool row, bool col)
   {
      int origSize = original.mSize;
      mSize = origSize / 2;
      
      int rRow = 0;
      int rCol = 0;
      // Calculate the quadrant index limits based off of the input row and col
      int qRowMin = row * (origSize / 2);
      int qRowMax = ((row + 1) * (origSize / 2));
      int qColMin = col * (origSize / 2);
      int qColMax = ((col + 1) * (origSize / 2));
      
      if (col)
      {
         colAlloc = true;
         mRows = new T*[mSize];
         // Copy pointers from the beginning of the right half of the top/bottom half...
         for (int qRow = qRowMin; qRow < qRowMax; ++qRow, ++rRow)
         {
            mRows[rRow] = &(original[qRow][qColMin]);
         }
      }
      else
      {
         colAlloc = false;
         mRows = &(original.mRows[qRowMin]);
      }
      rowAlloc = false;
   }
   
   // Starting point for thread creation
   static int thread_Start;
   // Stopping point for thread creation
   static int thread_Stop;
   // Limit the number of simultaneous threads
   static std::mutex threadLimiter[100];
   static int maxThreads;
   static int threadCount;
   
   /**************************************************************************
    * this: m-matrix
    * need to pass in a*, a*, addA, b*, b*, addB
    * 
    *************************************************************************/
   void mult_Fast(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
   {
      // Take over the lock for this matrix.....
      std::lock_guard<std::mutex> lk(mMutex);
      started = true;
      // Allocate the memory and fill it with the specified data...
      if (a1 != NULL)
      {
         this->allocMath(*a0, *a1, addA);
      }
      else
      {
         //*this = *a0;
         this->allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         //matrixB = *b0;
         matrixB.allocCopy(*b0);
      }
      //mult_ThreadFarming(matrixB, NULL, computers, numComputers, port, true);
      mult(matrixB, true);
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //void mult(Matrix<T>& matrixB)
   void mult(Matrix<T>& matrixB, bool wrapped = false)
   {
      // Take over the lock for this matrix.....
      std::mutex* pMutex = &mMutex;
      if (wrapped)
      {
         //std::cerr << "TF: using wrapper lock!\n";
         pMutex = &mWMutex;
      }
      else
      {
         //std::cerr << "TF: Grabbing object lock!\n";
      }
      //std::lock_guard<std::mutex> lk(mMutex);
      std::lock_guard<std::mutex> lk(*pMutex);
      started = true;
      
      /**/
      if (mSize < 512 && mSize > 1 && mSize <= thread_Stop)
      {
         // This is, oddly enough, about 1 second faster for 2048x2048...
         Matrix<T> mA(*this);
         multStandard3(mA, matrixB);
         //multStandard4(matrixB);
      }
      else if (mSize > 1)
      /*/
      if (mSize > 1)
      /**/
      //if (mSize > 1)
      {
         std::thread t[16];
         
         // Four quadrants for each matrix being multiplied
         Matrix<T> a00(*this, 0, 0);
         Matrix<T> a01(*this, 0, 1);
         Matrix<T> a10(*this, 1, 0);
         Matrix<T> a11(*this, 1, 1);
         Matrix<T> b00(matrixB, 0, 0);
         Matrix<T> b01(matrixB, 0, 1);
         Matrix<T> b10(matrixB, 1, 0);
         Matrix<T> b11(matrixB, 1, 1);
         
         // Temporary Matrices to hold the 7 multiplication results
         /*/
         Matrix<T> m1(mSize / 2);
         Matrix<T> m2(mSize / 2);
         Matrix<T> m3(mSize / 2);
         Matrix<T> m4(mSize / 2);
         Matrix<T> m5(mSize / 2);
         Matrix<T> m6(mSize / 2);
         Matrix<T> m7(mSize / 2);
         /*/
         // Initialize to the left side of the multiplication...
         //Matrix<T> m1(a00, a11, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m2(a10, a11, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m3(a00            ); // Make a copy...
         //Matrix<T> m4(a11            ); // Make a copy...
         //Matrix<T> m5(a00, a01, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m6(a10, a00, false); // Create new object, subtracting 2nd from 1st
         //Matrix<T> m7(a01, a11, false); // Create new object, subtracting 2nd from 1st
         Matrix<T> m1(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m2(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m3(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m4(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m5(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m6(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m7(mSize / 2, false); // Declare matrix, but do not allocate
         /**/
         
         // Get the 7 multiplication results
         // m1 = (a00 + a11) * (b00 + b11);
         // m2 = (a10 + a11) *  b00;
         // m3 =  a00        * (b01 - b11);
         // m4 =  a11        * (b10 - b00);
         // m5 = (a00 + a01) *  b11;
         // m6 = (a10 - a00) * (b00 + b01);
         // m7 = (a01 - a11) * (b10 + b11);
         Matrix<int>* null = NULL;
         
         // Split for the thread number optimization
         if (mSize > thread_Start)
         {
            // Matrix is too large. Slow down to save memory...
            //m1.mult_wrapper((b00 + b11), null);
            //m2.mult_wrapper((b00)      , null);
            //m3.mult_wrapper((b01 - b11), null);
            //m4.mult_wrapper((b10 - b00), null);
            //m5.mult_wrapper((b11)      , null);
            //m6.mult_wrapper((b00 + b01), null);
            //m7.mult_wrapper((b10 + b11), null);
            //m1.mult(b1, null);
            //m2.mult(b2, null);
            //m3.mult(b3, null);
            //m4.mult(b4, null);
            //m5.mult(b5, null);
            //m6.mult(b6, null);
            //m7.mult(b7, null);
            m1.mult_Fast(&a00, &a11 , true , &b00, &b11 , true );
            m2.mult_Fast(&a10, &a11 , true , &b00,  null, false);
            m3.mult_Fast(&a00,  null, false, &b01, &b11 , false);
            m4.mult_Fast(&a11,  null, false, &b10, &b00 , false);
            m5.mult_Fast(&a00, &a01 , true , &b11,  null, false);
            m6.mult_Fast(&a10, &a00 , false, &b00, &b01 , true );
            m7.mult_Fast(&a01, &a11 , false, &b10, &b11 , true );
         }
         else if (mSize > thread_Stop)
         {
            /*/
            t[1] = std::thread(&Matrix<T>::mult, (a00 + a11), (b00 + b11), &m1);
            t[2] = std::thread(&Matrix<T>::mult, (a10 + a11), (b00)      , &m2);
            t[3] = std::thread(&Matrix<T>::mult, (a00)      , (b01 - b11), &m3);
            t[4] = std::thread(&Matrix<T>::mult, (a11)      , (b10 - b00), &m4);
            t[5] = std::thread(&Matrix<T>::mult, (a00 + a01), (b11)      , &m5);
            t[6] = std::thread(&Matrix<T>::mult, (a10 - a00), (b00 + b01), &m6);
            t[7] = std::thread(&Matrix<T>::mult, (a01 - a11), (b10 + b11), &m7);
            /*/
            //t[1] = std::thread(&Matrix<T>::mult, &m1, (b00 + b11), null);
            //t[2] = std::thread(&Matrix<T>::mult, &m2, (b00)      , null);
            //t[3] = std::thread(&Matrix<T>::mult, &m3, (b01 - b11), null);
            //t[4] = std::thread(&Matrix<T>::mult, &m4, (b10 - b00), null);
            //t[5] = std::thread(&Matrix<T>::mult, &m5, (b11)      , null);
            //t[6] = std::thread(&Matrix<T>::mult, &m6, (b00 + b01), null);
            //t[7] = std::thread(&Matrix<T>::mult, &m7, (b10 + b11), null);
            t[1] = std::thread(&Matrix<T>::mult_Fast, &m1, &a00, &a11 , true , &b00, &b11 , true );
            t[2] = std::thread(&Matrix<T>::mult_Fast, &m2, &a10, &a11 , true , &b00,  null, false);
            t[3] = std::thread(&Matrix<T>::mult_Fast, &m3, &a00,  null, false, &b01, &b11 , false);
            t[4] = std::thread(&Matrix<T>::mult_Fast, &m4, &a11,  null, false, &b10, &b00 , false);
            t[5] = std::thread(&Matrix<T>::mult_Fast, &m5, &a00, &a01 , true , &b11,  null, false);
            t[6] = std::thread(&Matrix<T>::mult_Fast, &m6, &a10, &a00 , false, &b00, &b01 , true );
            t[7] = std::thread(&Matrix<T>::mult_Fast, &m7, &a01, &a11 , false, &b10, &b11 , true );
            /**/
         //}
         //else if (mSize > (thread_Stop / 2))
         //{
         //   //(a00 + a11).mult_wrapper (b00 + b11, &m1);
         //   //(a10 + a11).mult_wrapper (b00      , &m2);
         //   //(a00)      .mult_wrapper (b01 - b11, &m3);
         //   //(a11)      .mult_wrapper (b10 - b00, &m4);
         //   //(a00 + a01).mult_wrapper (b11      , &m5);
         //   //(a10 - a00).mult_wrapper (b00 + b01, &m6);
         //   //(a01 - a11).mult_wrapper (b10 + b11, &m7);
         //   { std::unique_lock<std::mutex> lock(threadLimiter[threadCount++ % maxThreads]); t[1] = std::thread(&Matrix<T>::mult_wrapper_lock, (a00 + a11), (b00 + b11), &m1, std::move(lock));}
         //   { std::unique_lock<std::mutex> lock(threadLimiter[threadCount++ % maxThreads]); t[2] = std::thread(&Matrix<T>::mult_wrapper_lock, (a10 + a11), (b00)      , &m2, std::move(lock));}
         //   { std::unique_lock<std::mutex> lock(threadLimiter[threadCount++ % maxThreads]); t[3] = std::thread(&Matrix<T>::mult_wrapper_lock, (a00)      , (b01 - b11), &m3, std::move(lock));}
         //   { std::unique_lock<std::mutex> lock(threadLimiter[threadCount++ % maxThreads]); t[4] = std::thread(&Matrix<T>::mult_wrapper_lock, (a11)      , (b10 - b00), &m4, std::move(lock));}
         //   { std::unique_lock<std::mutex> lock(threadLimiter[threadCount++ % maxThreads]); t[5] = std::thread(&Matrix<T>::mult_wrapper_lock, (a00 + a01), (b11)      , &m5, std::move(lock));}
         //   { std::unique_lock<std::mutex> lock(threadLimiter[threadCount++ % maxThreads]); t[6] = std::thread(&Matrix<T>::mult_wrapper_lock, (a10 - a00), (b00 + b01), &m6, std::move(lock));}
         //   { std::unique_lock<std::mutex> lock(threadLimiter[threadCount++ % maxThreads]); t[7] = std::thread(&Matrix<T>::mult_wrapper_lock, (a01 - a11), (b10 + b11), &m7, std::move(lock));}
         }
         else if (mSize > 512)
         {
            /*/
            (a00 + a11).mult_wrapper (b00 + b11, &m1);
            (a10 + a11).mult_wrapper (b00      , &m2);
            (a00)      .mult_wrapper (b01 - b11, &m3);
            (a11)      .mult_wrapper (b10 - b00, &m4);
            (a00 + a01).mult_wrapper (b11      , &m5);
            (a10 - a00).mult_wrapper (b00 + b01, &m6);
            (a01 - a11).mult_wrapper (b10 + b11, &m7);
            /*/
            //m1.mult_wrapper (b00 + b11, null);
            //m2.mult_wrapper (b00      , null);
            //m3.mult_wrapper (b01 - b11, null);
            //m4.mult_wrapper (b10 - b00, null);
            //m5.mult_wrapper (b11      , null);
            //m6.mult_wrapper (b00 + b01, null);
            //m7.mult_wrapper (b10 + b11, null);
            m1.mult_Fast(&a00, &a11 , true , &b00, &b11 , true );
            m2.mult_Fast(&a10, &a11 , true , &b00,  null, false);
            m3.mult_Fast(&a00,  null, false, &b01, &b11 , false);
            m4.mult_Fast(&a11,  null, false, &b10, &b00 , false);
            m5.mult_Fast(&a00, &a01 , true , &b11,  null, false);
            m6.mult_Fast(&a10, &a00 , false, &b00, &b01 , true );
            m7.mult_Fast(&a01, &a11 , false, &b10, &b11 , true );
            /**/
         }
         else
         {
            //cerr << "In bad branch!\n";
            // This is entered at one level, and it appears to work faster.
            /*/
            m1.reallocate();
            m2.reallocate();
            m3.reallocate();
            m4.reallocate();
            m5.reallocate();
            m6.reallocate();
            m7.reallocate();
            (a00 + a11).multStandard (b00 + b11, m1);
            (a10 + a11).multStandard (b00      , m2);
            (a00)      .multStandard (b01 - b11, m3);
            (a11)      .multStandard (b10 - b00, m4);
            (a00 + a01).multStandard (b11      , m5);
            (a10 - a00).multStandard (b00 + b01, m6);
            (a01 - a11).multStandard (b10 + b11, m7);
            /*/
            //cerr << "you got me!!\n";
            m1.multStandard2(&a00, &a11 , true , &b00, &b11 , true );
            m2.multStandard2(&a10, &a11 , true , &b00,  null, false);
            m3.multStandard2(&a00,  null, false, &b01, &b11 , false);
            m4.multStandard2(&a11,  null, false, &b10, &b00 , false);
            m5.multStandard2(&a00, &a01 , true , &b11,  null, false);
            m6.multStandard2(&a10, &a00 , false, &b00, &b01 , true );
            m7.multStandard2(&a01, &a11 , false, &b10, &b11 , true );
            /**/
         }
         // Clear out allocated memory....
         //b00.erase();
         //b01.erase();
         //b10.erase();
         //b11.erase();
         //// We don't need matrixB data anymore. Erase it.
         //matrixB.erase();
         
         /**/
         //if (mSize > (thread_Stop / 2))
         if (mSize > thread_Stop && mSize <= thread_Start)
         {
            //t[1].join();
            //t[2].join();
            //t[3].join();
            //t[4].join();
            //t[5].join();
            //t[6].join();
            //t[7].join();
         t[12] = std::thread(&Matrix<T>::op00_11_con, &a00, std::ref(m1), std::ref(m4), std::ref(m5), std::ref(m7), t, 1, 4, 5, 7);
         t[13] = std::thread(&Matrix<T>::op01_10_con, &a01, std::ref(m3), std::ref(m5), t, 3, 5);
         t[14] = std::thread(&Matrix<T>::op01_10_con, &a10, std::ref(m2), std::ref(m4), t, 2, 4);
         t[15] = std::thread(&Matrix<T>::op00_11_con, &a11, std::ref(m1), std::ref(m3), std::ref(m2), std::ref(m6), t, 1, 3, 2, 6);
         t[12].join();
         t[13].join();
         t[14].join();
         t[15].join();
         }
         else
         {
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         //a00 = m1 + m4 - m5 + m7;
         //a01 = m3 + m5;
         //a10 = m2 + m4;
         //a11 = m1 + m3 - m2 + m6;
         a00.op00_11(m1, m4, m5, m7);
         a01.op01_10(m3, m5);
         a10.op01_10(m2, m4);
         a11.op00_11(m1, m3, m2, m6);
         }
         /*/
         t[12] = std::thread(&Matrix<T>::op00_11_con, &a00, std::ref(m1), std::ref(m4), std::ref(m5), std::ref(m7), t, 1, 4, 5, 7);
         t[13] = std::thread(&Matrix<T>::op01_10_con, &a01, std::ref(m3), std::ref(m5), t, 3, 5);
         t[14] = std::thread(&Matrix<T>::op01_10_con, &a10, std::ref(m2), std::ref(m4), t, 2, 4);
         t[15] = std::thread(&Matrix<T>::op00_11_con, &a11, std::ref(m1), std::ref(m3), std::ref(m2), std::ref(m6), t, 1, 3, 2, 6);
         t[12].join();
         t[13].join();
         t[14].join();
         t[15].join();
         /**/
         
         // The above will re-write matrixA (calling object)
         // Reassemble the quadrants into a single whole
         //if (result != NULL)
         //{
         //   *result = Matrix(a00, a01, a10, a11);
         //}
      }
      else
      {
         // Assume a matrix of size 1
         //if (result != NULL)
         //{
         //   *result[0][0] = mRows[0][0] * matrixB[0][0];
         //}
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void mult_wrapper(Matrix<T> matrixB, Matrix<T>* result)
   {
      this->mult(matrixB, result);
      //this->mult(matrixB);
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //void mult_wrapper_lock(Matrix<T>& matrixB, Matrix<T>* result = NULL, std::unique_lock<std::mutex>&& lock)
   //{
   //   this->mult(matrixB, result);
   //}
   
   /**************************************************************************
    * Standard matrix multiplication
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> multStandard(const Matrix<T>& matrixB, Matrix<T>& result)
   {
      started = true;
      for (int i = 0; i < mSize; ++i)
      {
         for (int j = 0; j < mSize; ++j )
         {
            result[i][j] = 0;
            for (int k = 0; k < mSize; ++k)
            {
               result[i][j] += (*this)[i][k] * matrixB[k][j];
            }
         }
      }
      
      return result;
   }
   
   /**************************************************************************
    * Standard matrix multiplication
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   //Matrix<T> multStandard(const Matrix<T>& matrixB, Matrix<T>& result) const
   //void multStandard2(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB, std::unique_lock<std::mutex>&& ctlMut)
   //void multStandard2(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB, int& startBlock, std::mutex& mut)
   void multStandard2(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
   {
      //if (!mMutex.try_lock())
      //{
      //   std::cerr << Red << "MS2: FAILED TO LOCK!!!!!" << RCol << std::endl;
      //}
      //else
      //{
      //   mMutex.unlock();
      //}
      // Take over the lock for this matrix.....
      std::lock_guard<std::mutex> lk(mMutex);
      started = true;
      //{
      //std::lock_guard<std::mutex> stLk(mut);
      //--startBlock;
      //}
      //ctlMut.unlock();
      // Allocate the memory and fill it with the specified data...
      this->reallocate();
      Matrix<T> matrixA(mSize, false); // Declare but don't allocate
      if (a1 != NULL)
      {
         matrixA.allocMath(*a0, *a1, addA);
      }
      else
      {
         matrixA.allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         matrixB.allocCopy(*b0);
      }
      //bool del_a1 = false;
      //bool del_b1 = false;
      //if (a1 == NULL)
      //{
      //   a1 = new Matrix<T>(mSize, false);
      //   a1->allocZero();
      //   del_a1 = true;
      //}
      //if (b1 == NULL)
      //{
      //   b1 = new Matrix<T>(mSize, false);
      //   b1->allocZero();
      //   del_b1 = true;
      //}
      for (int i = 0; i < mSize; ++i)
      {
         for (int j = 0; j < mSize; ++j )
         {
            mRows[i][j] = 0;
            //int a;
            //int b;
            for (int k = 0; k < mSize; ++k)
            {
               //a = addA ? ((*a0)[i][k] + (*a1)[i][k]) : ((*a0)[i][k] - (*a1)[i][k]);
               //b = addB ? ((*b0)[k][j] + (*b1)[k][j]) : ((*b0)[k][j] - (*b1)[k][j]);
               //mRows[i][j] += a * b;
               mRows[i][j] += matrixA[i][k] * matrixB[k][j];
            }
         }
      }
      //if (del_a1)
      //{
      //   delete a1;
      //}
      //if (del_b1)
      //{
      //   delete b1;
      //}
      //std::cerr << "Exiting mult_standard2...\n";
      //return result;
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void multStandard3(const Matrix<T>& matrixA, const Matrix<T>& matrixB)
   {
      started = true;
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = 0;
            for (int k = 0; k < mSize; ++k)
            {
               mRows[i][j] += matrixA.mRows[i][k] * matrixB.mRows[k][j];
            }
         }
      }
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void multStandard4(const Matrix<T>& matrixB)
   {
      started = true;
      T** rows;
      rows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         rows[i] = new T[mSize];
         for (int j = 0; j < mSize; j++)
         {
            rows[i][j] = 0;
            for (int k = 0; k < mSize; ++k)
            {
               rows[i][j] += mRows[i][k] * matrixB.mRows[k][j];
            }
         }
      }
      for (int i = 0; i < mSize; i++)
      {
         delete [] mRows[i];
         mRows[i] = rows[i];
      }
      delete [] rows;
   }
};

// Initialize the static variables for Matrix
template <class T>
int Matrix<T>::thread_Start = 8192;
template <class T>
int Matrix<T>::thread_Stop = 0;
template <class T>
std::mutex Matrix<T>::threadLimiter[100];
template <class T>
int Matrix<T>::maxThreads = 100;
template <class T>
int Matrix<T>::threadCount = 0;

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
   int thread_Stop;

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
      return 1;
   }   

   Matrix<int> matrixA(size);
   Matrix<int> matrixB(size);
   if (size < 8192)
   {
      matrixA.thread_Start = size;
      matrixA.thread_Stop = thread_Stop;
   }
   else
   {
      matrixA.thread_Stop = 512;
      matrixA.thread_Start = matrixA.thread_Stop * 4;
   }
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
   //Matrix<int> result(size);
   //matrixA.mult(matrixB, result);
   //matrixA.mult(matrixB, NULL);
   matrixA.mult(matrixB);
   //matrixA.mult(matrixB);
   //cout << result;
   //cerr << matrixA;
   cout << matrixA;

   return 0;
}
