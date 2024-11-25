#include <iostream>
#include <pthread.h>
#include <functional>
#include <vector>
#include <mutex>

#include <chrono>

using namespace std;

//function for parallelizing single for loop
void parallel_for(int low, int high, function<void(int)> &&lambda, int numThreads) {
    auto start = chrono::high_resolution_clock::now();//start timer

    //calculate chunk size
    int chunk_size = high-low/numThreads;
    //vector to hold the threads
    vector<pthread_t> threads(numThreads);
    mutex lmutex;//mutex for synchronization

    //structure to hold thread data
    struct Data{
        int low; //low index
        int high;//high index
        function<void(int)> lambda;//lambda function
        mutex* smutex;//mutex for synchronization
    };

    // helper function for single loop parallelization
    //lambda function is passed as argument
    auto helper = [](void* arg) -> void* {
        Data* data = static_cast<Data*>(arg);
        //executr lambda function for each index means each thread
            lock_guard<mutex> lock(*data->smutex);
        for (int i=data->low;i<data->high;i++) {
            data->lambda(i);
        }
        return nullptr;//return null
    };

    //vector to hold thread data
    vector<Data> threadData(numThreads);

    //creating threads
    for (int i=0; i<numThreads; i++) {
        int threadLow = low + i * chunk_size;//calculate low index for each thread
        int threadHigh;//initialize high index for each thread
        if(i==numThreads-1){
            threadHigh = high;//if last thread then high index is equal to total high index
        } else {
            threadHigh = threadLow + chunk_size;//calculate high index for each thread
        }
        threadData[i] = {threadLow, threadHigh, lambda, &lmutex};//store low, high and lambda function in thread data
        //create thread
        pthread_create(&threads[i], nullptr, helper, &threadData[i]);
    }

    //make the thread wait for the completion of other threads
    for (int i = 0;i<numThreads;i++) {
        pthread_join(threads[i], nullptr);
    }

    //end timer
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end-start);//calculate duration
    cout<<"Total execution time for 1D parallel_for: "<<duration.count()<<" microseconds\n";
}

// function for parallelizing nested for loops
void parallel_for(int low1, int high1, int low2, int high2, function<void(int, int)> lambda, int numThreads) {
    int outerRange = high1-low1;//calculate outer range
    auto start = chrono::high_resolution_clock::now();//start timer

    int elements = (high1-low1)*(high2-low2);//calculate total number of elements/range
    int chunk_size = elements/numThreads;//calculate chunk size
    mutex lmutex;//mutex for synchronization
    vector<pthread_t> threads(numThreads);//vector to hold threads

    // structure to hold thread data
    struct Data {
        int low1; 
        int low2;
        int outerRange;
        int threadLow;
        int threadHigh;
        function<void(int, int)> lambda;
            mutex* smutex;
    };

    //helper function for nested loop parallelization
    auto helper = [](void* arg) -> void* {
        //cast the argument to Data type
        Data* data = static_cast<Data*>(arg);
        //execute lambda function for each index
        for (int i = data->threadLow; i<data->threadHigh; i++) {
            int x = data->low1 + (i % (data->outerRange));
            int y = data->low2 + (i / (data->outerRange));
            lock_guard<mutex> lock(*data->smutex);
            data->lambda(x, y);
        }
        return nullptr;//return null
    };

    vector<Data> threadData(numThreads);//vector to hold thread data

    //create threads
    for (int i = 0; i<numThreads; i++) {
        int threadLow = i*chunk_size;//calculate low index for each thread
        int threadHigh;//initialize high index for each thread
        if(i==numThreads-1){
            threadHigh = elements;//if last thread then high index is equal to total elements
        }
        else {
            threadHigh = threadLow+chunk_size;//else calculate high index for each thread
        }
        threadData[i] = {low1, low2, outerRange, threadLow, threadHigh, lambda, &lmutex};//store low1, low2, outerRange, low and high index and lambda function in thread data
        //create thread
        pthread_create(&threads[i], nullptr, helper, &threadData[i]);
    }

    //make the thread wait for the completion of other threads
    for (int i = 0; i<numThreads; i++) {
        pthread_join(threads[i], nullptr);
    }

    auto end = chrono::high_resolution_clock::now();//end timer
    auto duration = chrono::duration_cast<chrono::microseconds>(end-start);//calculate duration
    std::cout << "Total execution time for 2D parallel_for: " << duration.count() << " microseconds\n";
}

int user_main(int argc, char **argv);

/* Demonstration on how to pass lambda as parameter.
 * "&&" means r-value reference. You may read about it online.
 */
void demonstration(std::function<void()> && lambda) {
  lambda();
}

int main(int argc, char **argv) {
  /* 
   * Declaration of a sample C++ lambda function
   * that captures variable 'x' by value and 'y'
   * by reference. Global variables are by default
   * captured by reference and are not to be supplied
   * in the capture list. Only local variables must be 
   * explicity captured if they are used inside lambda.
   */
  int x=5,y=1;
  // Declaring a lambda expression that accepts void type parameter
  auto /*name*/ lambda1 = /*capture list*/[/*by value*/ x, /*by reference*/ &y](void) {
    /* Any changes to 'x' will throw compilation error as x is captured by value */
    y = 5;
    std::cout<<"====== Welcome to Assignment-"<<y<<" of the CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  // Executing the lambda function
  demonstration(lambda1); // the value of x is still 5, but the value of y is now 5

  int rc = user_main(argc, argv);
 
  auto /*name*/ lambda2 = [/*nothing captured*/]() {
    std::cout<<"====== Hope you enjoyed CSE231(A) ======\n";
    /* you can have any number of statements inside this lambda body */
  };
  demonstration(lambda2);
  return rc;
}

#define main user_main

