#include <iostream>

#define N 5

int A[N] = {54,-5,12,5,3};


void selection_sort(int *A, const int n)
{
   int i, j, min_idx;
   for(i = 0; i < n - 1; i++)
   {
       min_idx = i;
       for(j = i + 1; j < n; j++)
       {
           if(A[j] < A[min_idx])
                min_idx = j;
       }
       int tmp = A[i];
       A[i] = A[min_idx];
       A[min_idx] = tmp;
   }
}

int main()
{
    selection_sort(A,N);

    for(int i = 0; i < N; i++)
        std::cout << A[i] << std::endl;


    return 0;
}