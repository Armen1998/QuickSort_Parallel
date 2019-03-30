#include <cstdlib> 
#include <ctime> 
#include <iostream>
#include <stdio.h>     
#include <stdlib.h>    
#include <mpi.h>

using namespace std;

int* MergeArrays(int* arr1, int* arr2, int m, int n)
{
	int i, j, k;
	i = 0;
	j = 0;
	k = 0;
	int* result = new int[m + n];
	while (i < m && j < n) {
		if (arr1[i] <= arr2[j]) {
			result[k] = arr1[i];
			i++;
		}
		else {
			result[k] = arr2[j];
			j++;
		}
		k++;
	}
	if (i < m) {
		for (int p = i; p < m; p++) {
			result[k] = arr1[p];
			k++;
		}
	}
	else {
		for (int p = j; p < n; p++) {
			result[k] = arr2[p];
			k++;
		}
	}
	return result;

}

void QuickSort(int* arr, int left, int right) {	
		int i = left, j = right;
		int tmp;
		srand((unsigned)time(0));
		int p = rand() % (right - left) + left;
		int pivot = arr[p];

		while (i <= j) {
			while (arr[i] < pivot)
				i++;
			while (arr[j] > pivot)
				j--;
			if (i <= j) {
				tmp = arr[i];
				arr[i] = arr[j];
				arr[j] = tmp;
				i++;
				j--;
			}
		};

		if (left < j)
			QuickSort(arr, left, j);
		if (i < right)
			QuickSort(arr, i, right);
}

void main(int argc, char* argv[]) {
	int rank, size, N = 500000;
	MPI_Init(&argc, &argv);	
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status st;
	int eachProcSize = size != 1 ? N / (size - 1) : N;
	int additional = (N - (eachProcSize)* (size - 1));
	int additionalCount = 1;
	int* subArr; 
	int* arr = new int[N];
	int* resultArray = new int[N];
	int** newArray = new int*[size - 1];

	for (int i = 0; i < size - 1; i++)
	{
		if (i < additional)
		{
			newArray[i] = new int[eachProcSize + additional];
		}
		else
		{
			newArray[i] = new int[eachProcSize];
		}
	}

	if (rank > 0 && rank <= additional) {
		subArr = new int[eachProcSize + additionalCount];
	}
	else
	{
		subArr = new int[eachProcSize];
	}

	if (rank == 0) {
		srand((unsigned)time(0));

		for (int i = 0; i < N; i++) {
			arr[i] = (rand() % 1000) + 1;
		}
	}

	if (rank == 0)
	{
		int count = 0;
		
		for (int i = 1; i < size; i++) {
			MPI_Send(arr + count,
				i <= additional ? eachProcSize + additionalCount : eachProcSize,
				MPI_INT, i, i, MPI_COMM_WORLD);

			i <= additional ? count += (eachProcSize + additionalCount) :
				count += eachProcSize;
		}
		
	}
	else
	{
		
		MPI_Recv(subArr, rank <= additional ? eachProcSize + additionalCount : eachProcSize,
			MPI_INT, 0, rank, MPI_COMM_WORLD, &st);
	}
			
	if (rank != 0) {

		QuickSort(subArr, 0, rank <= additional ? eachProcSize + additionalCount - 1 : eachProcSize - 1);
	}

	if (rank != 0)
	{

		MPI_Send(subArr, rank <= additional ? eachProcSize + additionalCount : eachProcSize,
			MPI_INT, 0, rank, MPI_COMM_WORLD);

	}
	else
	{
		for (int i = 1; i < size; i++)
		{
			MPI_Recv(newArray[i - 1], i <= additional ? eachProcSize + additionalCount : eachProcSize,
				MPI_INT, i, i, MPI_COMM_WORLD, &st);
		}
	}
	int count = 0;
	if (rank == 0 && size != 1) {

		if (size == 2)
		{
			resultArray = newArray[0];
		}
		else
		{
			resultArray = MergeArrays(newArray[0], newArray[1], additional >= 1 ? eachProcSize + additionalCount : eachProcSize, additional >= 2 ? eachProcSize + additionalCount : eachProcSize);

			if (additional == 0) {
				count = 2 * eachProcSize;
			}
			else
			{
				count += additional == 1 ? 2 * eachProcSize + additionalCount : 2 * (eachProcSize + additionalCount);
			}

			for (int i = 2; i < size - 1; i++)
			{
				resultArray = MergeArrays(resultArray, newArray[i], count, i < additional ? eachProcSize + additionalCount : eachProcSize);

				if (i < additional)
					count += eachProcSize + additionalCount;
				else count += eachProcSize;

			}
		}
	}

	delete resultArray, arr, subArr;
	if (rank == 0) {
		for (int i = 0; i < size - 1; i++)
			delete newArray[i];
	}
	MPI_Finalize();
}

