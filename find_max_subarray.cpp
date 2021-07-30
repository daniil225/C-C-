 #include <iostream>
#include <math.h>

using namespace std;

struct MaxSubarray
{
    int low;
    int high;
    int max_sum;
};

/*Поиск паксимального подмассива за время O(n)*/

MaxSubarray Find_max_crossing_subarray(int A[], int low, int mid, int high)
{
    MaxSubarray Subarray;
    int sum = 0;
    int max_left; int max_right;
    int left_sum = -1000000; int right_sum = -1000000;
    
    for(int i = mid; i > 0; i--)
    {
        sum = sum + A[i];
        if(sum > left_sum)
        {
            left_sum = sum;
            max_left = i;
        }
    }

    sum = 0;

    for(int j = mid+1; j < high; j++)
    {
        sum = sum + A[j];
        if(sum > right_sum)
        {
            right_sum = sum;
            max_right = j;
        }
    }
    Subarray.low = max_left;
    Subarray.high = max_right;
    Subarray.max_sum = left_sum + right_sum;
    return Subarray;
}

/*Выполнение поиска максимального подмассива за время О(nlg(n))*/

MaxSubarray Find_maximum_subarray(int A[], int low, int high)
{
    if(high == low)
    {
        MaxSubarray Subarray;
        Subarray.low = low;
        Subarray.high = high;
        Subarray.max_sum = A[low];
        return Subarray;
    }
    else
    {
        int mid = floor((low + high)/2);
        MaxSubarray Left = Find_maximum_subarray(A,low, mid);
        MaxSubarray Right = Find_maximum_subarray(A,mid + 1, high);
        MaxSubarray Crossing = Find_max_crossing_subarray(A, low, mid, high);

        if((Left.max_sum >= Right.max_sum) && (Left.max_sum >= Crossing.max_sum))
            return Left;
        else if((Right.max_sum >= Left.max_sum) && (Right.max_sum >= Crossing.max_sum))
            return Right;
        else
            return Crossing;
    }
    
}

 int main()
 {
     int A[16] = {13,-3,-25,20,-3,-16,-23,18,20,-7,12,-5,-22,15,-4,7};

    MaxSubarray Subarray = Find_maximum_subarray(A, 0,16);

    cout << Subarray.low << "  " << Subarray.high << "  " << Subarray.max_sum << endl;

     return 0;
 }