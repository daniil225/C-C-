/*
Алгоритм Винограда - Штрассена
 
A11 A12     B11 B12     C11 C12
A21 A22  *  B21 B22  =  C21 C22
 
S1 = A21 + A22
S2 = S1 - A11
S3 = A11 - A21
S4 = A12 - S2
S5 = B12 - B11
S6 = B22 - S5
S7 = B22 - B12
S8 = S6 - B21
 
P1 = S2 * S6
P2 = A11 * B11
P3 = A12 * B21
P4 = S3 * S7
P5 = S1 * S5
P6 = S4 * B22
P7 = A22 * S8
 
T1 = P1 + P2
T2 = T1 + P4
 
C11 = P2 + P3
C12 = T1 + P5 + P6
C21 = T2 - P7
C22 = T2 + P5
*/
 
//копируем в линейно размещенную матрицу a квадрант матрицы b
//начинаем с элемента b[ib][[jb]
//n - размерность матрицы b

#include <iostream>
using namespace std;
void copy(int *a, int *b, int ib, int jb, int n)
{
    int i, j, k;
    int imax = ib+n/2;      //до какого индекса строки копируем
    int jmax = jb+n/2;      //аналогично по столбцам
 
    for(k=0,i=ib; i<imax; i++)
    {
        for(j=jb; j<jmax; j++)
        {
            a[k++] = b[i*n+j];  //в матрице a элементы идут подряд
        }
    }
}
 
//копируем в квадрант матрицы a линейно размещенную матрицу b
//начинаем с элемента a[ia][[ja]
//n - размерность матрицы a
void copyback(int *a, int ia, int ja, int *b, int n)
{
    int i, j, k;
    int imax = ia+n/2;
    int jmax = ja+n/2;
 
    for(k=0,i=ia; i<imax; i++)
    {
        for(j=ja; j<jmax; j++)
        {
            a[i*n+j] = b[k++];
        }
    }
}
 
//складываем линейно размещенные матрицы c = a + b
void add(int *c, int *a, int *b, int n)
{
    for(int i=0; i<n*n; i++)
        c[i] = a[i] + b[i];
}
 
//вычитаем линейно размещенные матрицы c = a - b
void sub(int *c, int *a, int *b, int n)
{
    for(int i=0; i<n*n; i++)
        c[i] = a[i] - b[i];
}
 
//обычное матричное умножение c = a * b
void mul_normal(int *c, int *a, int *b, int n)
{
    int i, j, k;
    for(i=0; i<n; i++)
        for(j=0; j<n; j++)
        {
            c[i*n+j] = 0;
            for(k=0; k<n; k++)
                c[i*n+j] += a[i*n+k] * b[k*n+j];
        }
}
 
//умножение алгоритмом Винограда - Штрассена (модификация алгоритма Штрассена)
//c = a * b
void mul(int *c, int *a, int *b, int n)
{
    //при значениях меньше некоторого, выгоднее использовать обычное умножение
    //для простоты отладки стоит 2. Имеет смысл задать около 32
    if (n <= 1)
        mul_normal(c, a, b, n);         //обычное умножение
    else
    {
        int h = n/2;                    //делим размерность пополам
        int *M = new int[h*h*29];       //выделяем память под 29 вспомогательные матрицы
                                        //рассматриваем для убыстрения, как одномерный массив
 
//0-3: четверти матрицы A
        copy(&M[0], a, 0, 0, n);                    //M[0] = A11
        copy(&M[h*h], a, 0, h, n);                  //M[1] = A12
        copy(&M[2*h*h], a, h, 0, n);                //M[2] = A21
        copy(&M[3*h*h], a, h, h, n);                //M[3] = A22
 
//4-7: четверти матрицы B
        copy(&M[4*h*h], b, 0, 0, n);                //M[4] = B11
        copy(&M[5*h*h], b, 0, h, n);                //M[5] = B12
        copy(&M[6*h*h], b, h, 0, n);                //M[6] = B21
        copy(&M[7*h*h], b, h, h, n);                //M[7] = B22
 
//8-15: S1 - S8
        add(&M[8*h*h], &M[2*h*h], &M[3*h*h], h);    //M[8] = S1 = A21 + A22
        sub(&M[9*h*h], &M[8*h*h], &M[0], h);        //M[9] = S2 = S1 - A11
        sub(&M[10*h*h], &M[0], &M[2*h*h], h);       //M[10] = S3 = A11 - A21
        sub(&M[11*h*h], &M[h*h], &M[9*h*h], h);     //M[11] = S4 = A12 - S2
        sub(&M[12*h*h], &M[5*h*h], &M[4*h*h], h);   //M[12] = S5 = B12 - B11
        sub(&M[13*h*h], &M[7*h*h], &M[12*h*h], h);  //M[13] = S6 = B22 - S5
        sub(&M[14*h*h], &M[7*h*h], &M[5*h*h], h);   //M[14] = S7 = B22 - B12
        sub(&M[15*h*h], &M[13*h*h], &M[6*h*h], h);  //M[15] = S8 = S6 - B21
 
//16-22: P1 - P7: рекурсивный вызов себя же для умножения
        mul(&M[16*h*h], &M[9*h*h], &M[13*h*h], h);  //M[16] = P1 = S2 * S6
        mul(&M[17*h*h], &M[0], &M[4*h*h], h);       //M[17] = P2 = A11 * B11
        mul(&M[18*h*h], &M[h*h], &M[6*h*h], h);     //M[18] = P3 = A12 * B21
        mul(&M[19*h*h], &M[10*h*h], &M[14*h*h], h); //M[19] = P4 = S3 * S7
        mul(&M[20*h*h], &M[8*h*h], &M[12*h*h], h);  //M[20] = P5 = S1 * S5
        mul(&M[21*h*h], &M[11*h*h], &M[7*h*h], h);  //M[21] = P6 = S4 * B22
        mul(&M[22*h*h], &M[3*h*h], &M[15*h*h], h);  //M[22] = P7 = A22 * S8
 
//23-24: T1, T2
        add(&M[23*h*h], &M[16*h*h], &M[17*h*h], h); //M[23] = T1 = P1 + P2
        add(&M[24*h*h], &M[23*h*h], &M[19*h*h], h); //M[24] = T2 = T1 + P4
 
//25-28: четверти результирующей матрицы C
        add(&M[25*h*h], &M[17*h*h], &M[18*h*h], h); //M[25] = C11 = P2 + P3
        add(&M[26*h*h], &M[23*h*h], &M[20*h*h], h); //M[26] = C12 = T1 + P5
        add(&M[26*h*h], &M[26*h*h], &M[21*h*h], h); //M[26] = C12 += P6
        sub(&M[27*h*h], &M[24*h*h], &M[22*h*h], h); //M[27] = C21 = T2 - P7
        add(&M[28*h*h], &M[24*h*h], &M[20*h*h], h); //M[28] = C22 = T2 + P5
 
//копируем результат
        copyback(c, 0, 0, &M[25*h*h], n);           //C11 = M[25]
        copyback(c, 0, h, &M[26*h*h], n);           //C12 = M[26]
        copyback(c, h, 0, &M[27*h*h], n);           //C21 = M[27]
        copyback(c, h, h, &M[28*h*h], n);           //C22 = M[28]
        delete M;
    }
}
 
int main()
{
    int a[2][2] =
    {
        {1,3},
        {7,5}
    };
    int b[2][2] =
    {
        {6,8},
        {4,2}
    };
    int c[2][2];
    int c1[2][2];
 
//умножаем методом Винограда - Штрассена
    mul((int*)c, (int*)a, (int*)b, 2);
//для контроля - обычным умножением
    mul_normal((int*)c1, (int*)a, (int*)b, 2);

    for(int i = 0; i < 2; i++)
    {
        for(int j = 0; j < 2; j++)
        {
            cout << c1[i][j] << " ";
        }
        cout << endl;
    }

    return 0;
}