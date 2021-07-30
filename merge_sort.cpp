#include <iostream>

using namespace std;

void Merge(int A[], int p, int q, int r)
{
    int n1,n2,i,j,k;

    n1 = q - p + 1;
    n2 = r - q;
    int L[n1], R[n2];

    for(i = 0; i < n1; i++)
        L[i] = A[p + i];
    for(j = 0; j < n2; j++)
        R[j] = A[q + j + 1];
    
    i = 0;
    j = 0;

    for(k = p; i < n1 && j < n2; k++)
    {
        if(L[i] < R[j])
            A[k] = L[i++];
        else
            A[k] = R[j++];
    }

    while(i < n1)
    {
        A[k++] = L[i++];
    }
    while(j < n2)
    {
        A[k++] = R[j++];
    }
}

void Merge_sort(int A[], int p, int r)
{
    int q;
    if (p < r)
    {
        q = (r + p)/2;
        Merge_sort(A, p, q);
        Merge_sort(A, q + 1, r);
        Merge(A,p,q, r);
    }
}

void insertionsortRecursive(int A[], int n)
{
    if(n <= 1)
        return;
    
    insertionsortRecursive(A,n-1);
    int last = A[n - 1];
    int j = n - 2;

    while(j >= 0 && A[j] > last)
    {
        A[j + 1] = A[j];
        j--;
    }
    A[j + 1] = last;
}

void printArray(int A[], int n)
{
    int i;
    for(i = 0; i < n; i++)
        cout << A[i] << " ";
}

int binary_serch(int A[], int left, int right, int key)
{
    int midd = 0;
    while(1)
    {
        midd = (left + right)/2;
        if(key < A[midd])
            right = midd - 1;
        else if(key > A[midd])
            left = midd + 1;
        else
            return midd;

       if(left > right)
        return -1; 
    }
    
}

int main()
{
    int n = 5;
    int B[] = {45,34,223,12,-5};
    insertionsortRecursive(B, n);
    printArray(B,n);
    int v;
    cin >> v;
    int key = binary_serch(B, 0, n, v);
    cout << endl;
    cout << key << endl;
    return 0;
}