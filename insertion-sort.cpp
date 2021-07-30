#include <iostream>

#define N 15

void insert_sort(int *A, const int n, int params = 0)
{
    int i, j, key;
    for(j = 1; j < n; j++)
    {
        key = A[j];
        i = j - 1;
        if(params == 0)
        {
            while((i >= 0) && (A[i] > key))
            {
                A[i + 1] = A[i];
                i = i - 1;
            }
            A[i + 1] = key;
        }
        if(params == 1)
        {
            while((i >= 0) && (A[i] < key))
            {
                A[i + 1] = A[i];
                i = i - 1;
            }
            A[i + 1] = key;
        }
    }
}

int main()
{
    int A[N] = {5,1,3,5,7,8,10,12,0,34,67,12,23,100,5};

    insert_sort(A, N);
    for(int i = 0; i < N; i++)
        std::cout << A[i] << std::endl;
    
}